using Godot;
using System;
using System.IO.Ports;
using System.Text;

// Andre Koka - The Living Board Game - Fall 2023
// script to access c# while on the overworld, via the SerialManager Node
// primarily used to read/send serial messages during gameplay

public class SerialManager : Node
{
    byte[] received = new byte[26]; // stores current message from pico
    SerialPort port;

    // variables for parsed messages
	byte[] stats = new byte[16];
	byte p_class = 0;
	byte NFC_index = 0;

    // Called when the node enters the scene tree for the first time.
    public override void _Ready()
    {
        port = PlayerVars.port;
    }

    // Called every frame. 'delta' is the elapsed time since the previous frame.
    public override void _Process(float delta)
    {
        // receive and process serial messages (when required)
        if(PlayerVars.isLinux && port.BytesToRead > 0)
        {
            int i = read_msg();
            if(i != -1)
            {
                Godot.Node UI = GetNode("/root/Board/CanvasLayer/UI");
                UI.Call("set_gold", i, PlayerVars.gold[i]);
            }
        }
    }

    private void make_equip(byte speed, byte view, byte attack, byte defense, string text) // called from GDscript
    {
        var result = make_stats(speed, view, 0, 0, attack, defense);
        Random r = new Random();
        send_equip_msg(0, result, text, (byte)r.Next(0, 5), (byte)r.Next(0, 5)); // make and send message (random im and eq index)
    }

    // used to alter stats of player RFID tokens
    private void send_player_msg(byte NFC_index, byte[] stats) 
    {
        // create array to send (based on gage's docs)
        byte[] temp = new byte[2] {(byte)'W', (byte)NFC_index}; // assign NFC index here
        temp = Combine(temp, stats); // attach stats (16 bytes)

        // send over serial
        port.Write(temp, 0, 18); // this type of message is always 18 bytes
    }

    // used to create a new equipment token
    private void send_equip_msg(byte p_class, byte[] stats, string text, byte im_index, byte eq_index)
    {
        // create array to send (based on gage's docs)
        byte[] temp = new byte[2] {(byte)'W', (byte)6}; // assign NFC index here
        temp = Combine(temp, new byte[] {p_class}); // attach class (1 byte)
        temp = Combine(temp, stats); // attach stats (16 bytes)

        var temp2 = Encoding.ASCII.GetBytes(text);
        var temp3 = Combine(temp2, new byte[112-temp2.Length]); // text must be <= 111 bytes
        temp = Combine(temp, temp3); // attach text (112 bytes)

        temp = Combine(temp, new byte[] {im_index}); // attach image index (1 byte)
        temp = Combine(temp, new byte[] {eq_index}); // attach equipment index (1 byte)

        // send over serial
        port.Write(temp, 0, 133); // this type of message is always 133 bytes
    }

    // check for, read, and parse incoming messages from pi pico
    private int read_msg()
    {
        if(port.BytesToRead > 0)
        {
            GD.Print("receiving a serial message");
            port.Read(received, 0, 26); // messages from pico are always 26 bytes     

            // parse message
			if(received[0] != 'R')
				GD.Print("Error! Received message doesn't have R");
			NFC_index = received[1]; // 1 byte - NFC index
			p_class = received[9]; // 1 byte - class
			for(int i = 10; i < 26; i++) // 16 bytes - stats
				stats[i-10] = received[i];

			// process message contents
            if(NFC_index == 170) // 0xAA in decimal
            {
                GD.Print("error from pico");
                return -1;
            }
			else if(p_class == 0) // figure removed
			{
				PlayerVars.on_main_board[NFC_index] = 0;
			}
			else if(NFC_index == 4) // figure on final boss space
			{
				GD.Print("Figure on final event space!");
                this.GetTree().Quit();
			}
			else
			{
				PlayerVars.on_main_board[NFC_index] = 1; // figure placed
				PlayerVars.p_class[NFC_index] = p_class; // assign class
				// stats
				PlayerVars.speed[NFC_index] = stats[0]; // assign all player stats
				PlayerVars.view[NFC_index] = stats[1];
				PlayerVars.health[NFC_index] = stats[2];
				stats[3] = 0; // random white space required here
				PlayerVars.gold[NFC_index] = BitConverter.ToInt16(stats, 4);
				PlayerVars.defense[NFC_index] = stats[6];
				PlayerVars.attack[NFC_index] = stats[7];
			}
            return NFC_index;
        }
        return -1;
    }

    // to assemble the 16 bytes of stats
    private byte[] make_stats(byte speed, byte view, byte health, int gold, byte defense, byte attack) 
    {
        var temp = new Byte[1] {speed};
        temp = Combine(temp, new byte[] {view});
        temp = Combine(temp, new byte[] {health});

        // gold - convert to short, then to byte[]
        var new_gold = (Int16) gold;
        temp = Combine(temp, new byte[1]); // add extra white space
        temp = Combine(temp, BitConverter.GetBytes(new_gold));

        temp = Combine(temp, new byte[] {defense});
        temp = Combine(temp, new byte[] {attack});
        temp = Combine(temp, new byte[8]);
        return temp; // should be 16 byte stats array, with gap before gold
    }

    // function to append byte array to another byte array
    // adapted from: https://stackoverflow.com/questions/415291/best-way-to-combine-two-or-more-byte-arrays-in-c-sharp
    byte[] Combine(byte[] a1, byte[] a2)
    {
        byte[] ret = new byte[a1.Length + a2.Length];
        Array.Copy(a1, 0, ret, 0, a1.Length);
        Array.Copy(a2, 0, ret, a1.Length, a2.Length);
        return ret;
    }
}
