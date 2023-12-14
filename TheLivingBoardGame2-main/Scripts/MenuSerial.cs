using Godot;
using System;
using System.Diagnostics;

// Andre Koka - The Living Board Game - Fall 2023
// small script to allow use of C# on main menu

public class MenuSerial : Node
{
    Godot.Label label; 
    Godot.Button start;
    Godot.Button help;
    Godot.Button back;

    // Called when the node enters the scene tree for the first time.
    public override void _Ready()
    {
        label = GetNode<Godot.Label>("HelpLabel");
        start = GetNode<Godot.Button>("/root/Menu/StartGame");
        help = GetNode<Godot.Button>("/root/Menu/Help");
        back = GetNode<Godot.Button>("Back");
    }

    // Called every frame. 'delta' is the elapsed time since the previous frame.
    public override void _Process(float delta)
    {
        if(PlayerVars.isLinux)
        {
            // poll for any input for button press
            for(int i = 1; i < 5; i++)
            {
                var result = PlayerVars.RunCommandWithBash("redis-cli HGET user"+i.ToString()+" up", true);
                int res = Int32.Parse(result);
                if(res == 1)
                {  
                    PlayerVars.RunCommandWithBash("redis-cli HSET user"+i.ToString()+ " up 0", false);
                    if(!label.Visible)
                    {
                        start.EmitSignal("pressed"); // start game
                    }
                }

                result = PlayerVars.RunCommandWithBash("redis-cli HGET user"+i.ToString()+" down", true);
                res = Int32.Parse(result);
                if(res == 1)
                {  
                    PlayerVars.RunCommandWithBash("redis-cli HSET user"+i.ToString()+ " down 0", false);
                    if(!label.Visible)
                    {
                        help.EmitSignal("pressed"); // enter help screen
                    }
                    else
                    {
                        back.EmitSignal("pressed"); // exit help screen
                    }
                }
            }
        }
    }
}
