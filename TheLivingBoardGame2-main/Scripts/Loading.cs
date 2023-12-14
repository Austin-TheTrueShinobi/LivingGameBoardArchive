using Godot;
using System.IO.Ports;
using System;

// Andre Koka - The Living Board Game - Fall 2023
// script to control loading sequence, which is the first scene of the program

public class Loading : Node2D
{
	// global vars
	int remaining = 5; // controls length of start countdown
	bool start = false; // indicates when game is starting
	public PackedScene next_scene;
	Godot.Timer timer;

	// variables for parsed messages
	byte[] UID = new byte[7];
	byte[] stats = new byte[16];
	byte p_class = 0;
	byte NFC_index = 0;

	public static SerialPort port; // main serial port to be used
	byte[] received = new byte[26]; // stores received serial message
	byte[] s = new byte[1]; // stores byte 'S' as a byte array

    public override void _Ready() // called when node enters scene tree for first time
    {
		if(PlayerVars.isLinux)
		{
            port = PlayerVars.port; // get port from singleton
			GD.Print("sending S"); // required to start pi pico
			s[0] = 83; // 'S'
			port.Write(s, 0, 1);
			PlayerVars.reset_inputs();
		}
		next_scene = ResourceLoader.Load<PackedScene>("res://Scenes/Menu.tscn"); // prepare next scene (main menu)
		timer = GetNode<Godot.Timer>("Timer"); // get local timer node
    }

    public override void _Process(float delta)    // Called every frame. 'delta' is the elapsed time since the previous frame.
    {
		// poll for player figure
		if(PlayerVars.isLinux)
		{
			if (port.BytesToRead > 0)
			{
				port.Read(received, 0, 26); // read one msg from pi pico

				// parse message
				if(received[0] != 'R')
					GD.Print("Error! Received message doesn't have R");
				NFC_index = received[1]; // 1 byte - NFC index
				for(int i = 2; i < 9; i++) // 7 bytes - UID
					UID[i-2] = received[i];
				p_class = received[9]; // 1 byte - player class
				for(int i = 10; i < 26; i++) // 16 bytes - stats
					stats[i-10] = received[i];

				// process message contents
				if(p_class == 0) // figure removed
				{
					PlayerVars.on_main_board[NFC_index] = 0;
				}
				else if(NFC_index >= 4)
				{
					// player on final thing
					// does nothing in startup sequence
				}
				else
				{
					PlayerVars.on_main_board[NFC_index] = 1; // figure placed
					PlayerVars.p_class[NFC_index] = p_class; // assign class

					// assign UID
					for(int i = 0; i < 7; i++)
						PlayerVars.uid[NFC_index, i] = UID[i];
					string s = BitConverter.ToString(UID);
					GD.Print("redis-cli HSET user"+(NFC_index+1)+" UID "+s.Replace("-", ""));
					PlayerVars.RunCommandWithBash("redis-cli HSET user"+(NFC_index+1)+" UID "+s.Replace("-", ""), false);

					PlayerVars.player_num[NFC_index] = NFC_index; // assign player number (based on initial NFC index)

					// stats
					PlayerVars.speed[NFC_index] = stats[0]; // ACCOUNT FOR MAX VALUES
					PlayerVars.view[NFC_index] = stats[1];
					PlayerVars.health[NFC_index] = stats[2];
					stats[3] = 0; // manual white space for pico
					PlayerVars.gold[NFC_index] = BitConverter.ToInt16(stats, 4);
					PlayerVars.defense[NFC_index] = stats[6];
					PlayerVars.attack[NFC_index] = stats[7];
				}
			}
		}

		// poll for input (keyboard)
		if(Input.IsActionPressed("ui_right") && PlayerVars.connected[0]!=1) // dummy p1 joined
		{
			PlayerVars.connected[0] = 1;
			PlayerVars.num_players += 1;
		}
		if(Input.IsActionPressed("ui_left") && PlayerVars.connected[1]!=1) // dummy p2 joined
		{
			PlayerVars.connected[1] = 1;
			PlayerVars.num_players += 1;
		}
		if(Input.IsActionPressed("ui_up") && PlayerVars.connected[2]!=1) // dummy p3 joined
		{
			PlayerVars.connected[2] = 1;
			PlayerVars.num_players += 1;
		}
		if(Input.IsActionPressed("ui_down") && PlayerVars.connected[3]!=1) // dummy p4 joined
		{
			PlayerVars.connected[3] = 1;
			PlayerVars.num_players += 1;
		}

		// poll for input (buttons)
		if(PlayerVars.isLinux)
		{
			for(int i = 0; i < 4; i++)
			{
				var result = PlayerVars.RunCommandWithBash("redis-cli HGET user"+(i+1).ToString()+" left", true);
				int res = Int32.Parse(result);
				if(res == 1)
				{  
					PlayerVars.RunCommandWithBash("redis-cli HSET user"+(i+1).ToString()+ " left 0", false);
					PlayerVars.connected[i] = 1;
					GD.Print("player "+(i+1).ToString()+" connected");
				}
				if(PlayerVars.on_main_board[i] == 1 && PlayerVars.connected[i] == 1 && PlayerVars.tracking[i] == 0)
				{
					PlayerVars.num_players++; 
					PlayerVars.tracking[i] = 1;
				}
				else if ((PlayerVars.on_main_board[i] == 0 || PlayerVars.connected[i] == 0) && PlayerVars.tracking[i] == 1)
				{
					PlayerVars.num_players--;
					PlayerVars.tracking[i] = 0;
				}

			}
		}
		
		for(int i = 0; i < 4; i++) // make labels visible and update PlayerVars.counted
		{
			// if figure just placed on main board
			if(PlayerVars.on_main_board[i] == 1 && PlayerVars.counted[i] == 0)
			{
				Godot.Label label = GetNode<Godot.Label>("Conn"+(i+1));
				label.Visible = true;
				PlayerVars.counted[i] = 1;
				if(!PlayerVars.isLinux)
					PlayerVars.connected[i] = 1;
			}
			// if player just removed from main board
			else if(PlayerVars.on_main_board[i] == 0 && PlayerVars.counted[i] == 1)
			{
				Godot.Label label = GetNode<Godot.Label>("Conn"+(i+1));
				label.Visible = false;
				if(PlayerVars.counted[i] == 1 && PlayerVars.connected[i] == 1)
				{
					PlayerVars.counted[i] = 0;
					if(!PlayerVars.isLinux)
						PlayerVars.connected[i] = 0;
				}
			}

			// make labels visible
			if(PlayerVars.connected[i] == 1)
			{
				Godot.Label label = GetNode<Godot.Label>("Input"+(i+1));
				label.Visible = true;
			}
			else
			{
				Godot.Label label = GetNode<Godot.Label>("Input"+(i+1));
				label.Visible = false;
			}
		} 
		if(PlayerVars.num_players >= 2 && start == false) // start when >=2 players connected
		{
			timer.Start();
            start = true;
		}
		if(PlayerVars.num_players < 2 && start) // abort startup if <2 players
		{
			timer.Stop();
			start = false;
			remaining = 5;
			GetNode<Label>("Timing").Text= "";
		}
    }

    private void _on_Timer_timeout() // transition to main menu
	{
		remaining -= 1;
		GetNode<Label>("Timing").Text= "Starting in "+remaining.ToString()+"...";
		if(remaining <= 0)
		{
			GetTree().Root.AddChild(next_scene.Instance());
			SetProcess(false);
			QueueFree();
		}
	}

	private void PrintSerialPorts() // for debugging
	{
		string[] ports = SerialPort.GetPortNames();
        GD.Print("The following serial ports were found:");
        foreach(string port in ports)
        { GD.Print(port); }
	}
}
