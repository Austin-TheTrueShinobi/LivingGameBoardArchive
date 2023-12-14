using Godot;
using System;

// Andre Koka - The Living Board Game - Fall 2023
// script to control each player - one of the most important

public class Player : Area2D
{
	int gridsize = 16; // each grid tile is 16 pixels
	int moves_left;
	bool is_moving = false;
	int moving = 0;
	int delay = 50; // only poll player input every 50 frames
	Vector2 direction;

	Godot.RayCast2D ray;
	Godot.TileMap tilemap;
	Godot.Tween tween;
	Godot.Light2D light;
	Godot.Node board;

	int my_num = 0; // player number
	int on_main_board = 0; // is this player on the main board
	int is_my_turn = 0;

	[Export]
    private bool inputs_enabled = false;

	// called when node enters scene for the first time
	public override void _Ready()
	{
		// establish references to all nodes the player needs
		board = this.GetParent();
		ray = GetNode<Godot.RayCast2D>("RayCast2D");
		tilemap = GetNode<Godot.TileMap>("/root/Board/TileMap");

		tween = GetNode<Godot.Tween>("Tween");
		tween.Connect("tween_all_completed", this, nameof(_on_Tween_tween_all_completed));

		light = GetNode<Godot.Light2D>("Light2D");
	}

	// Called every frame. 'delta' is the elapsed time since the previous frame.
	public override void _Process(float delta)
	{
		light.TextureScale = PlayerVars.view[my_num-1]; // player view implementation
		on_main_board = PlayerVars.on_main_board[my_num-1];
		if(PlayerVars.isLinux)
		{
			if(on_main_board==1 && !is_moving  && is_my_turn == 1)
			{
				inputs_enabled = true;
			}
			else if (on_main_board==0 && !is_moving && is_my_turn == 1)
			{
				inputs_enabled = false;
			}
		}

		// poll for input (buttons) (for movement on overworld)
		if(inputs_enabled && moves_left>0 && PlayerVars.isLinux)
		{
			if(delay <= 0)
			{
				delay = 50;
				if(GetUp() == 1)
				{	move(new Vector2(0, -1)); }
				else if(GetDown() == 1)
				{	move(new Vector2(0, 1)); }
				else if(GetRight() == 1)
				{	move(new Vector2(1, 0)); }
				else if(GetLeft() == 1)
				{	move(new Vector2(-1, 0)); }
			}
			else
			{
				delay--;
			}
		}
	} 

	// poll for input (keyboard) (testing only)
	public override void _Input(InputEvent @event)
	{
		if(inputs_enabled && moves_left > 0)
		{
    		if (@event.IsActionPressed("ui_right"))
    		{	move(new Vector2(1, 0)); }
			if (@event.IsActionPressed("ui_left"))
    		{	move(new Vector2(-1, 0));  }
			if (@event.IsActionPressed("ui_up"))
    		{	move(new Vector2(0, -1));  }
			if (@event.IsActionPressed("ui_down"))
    		{	move(new Vector2(0, 1));  }
			if (@event.IsActionPressed("ui_focus_next"))
			{
				board.Call("tween_camera");
				var temp = board.Get("camera_zoomed");
				board.Set("camera_zoomed", !(bool)temp);
			}
		}
	}

	// move player on overworld in the given direction
	private void move(Vector2 dir)
	{
		ray.CastTo = dir*gridsize;
		ray.ForceRaycastUpdate();
		if(!ray.IsColliding()) // player shouldn't move through certain objects
		{
			tween.InterpolateProperty(this, "position", new Vector2(Position.x, Position.y), Position + dir*gridsize, 0.2f, Tween.TransitionType.Linear, Tween.EaseType.InOut);
			inputs_enabled = false; // no inputs allowed while moving
			is_moving = true;
			tween.Start();
			moves_left--;
		}
	}

	private void _on_Tween_tween_all_completed() // once movement is completed
	{
		inputs_enabled = true;
		is_moving = false;
	}

	private void _on_Player_area_entered(Area2D area)
	{
		if(area.IsInGroup("shop"))
		{
			// action to take when entering shop
		}
	}

	// custom input system - receive inputs from Redis
	public int GetUp() // get player's up input
	{
		var result = PlayerVars.RunCommandWithBash("redis-cli HGET user"+my_num+" up", true);
		PlayerVars.RunCommandWithBash("redis-cli HSET user"+my_num+" up 0", false);
		return Int32.Parse(result);
	}
	public int GetDown() // get player's down input
	{
		var result = PlayerVars.RunCommandWithBash("redis-cli HGET user"+my_num+" down", true);
		PlayerVars.RunCommandWithBash("redis-cli HSET user"+my_num+" down 0", false);
		return Int32.Parse(result);
	}
	public int GetRight() // get player's right input
	{
		var result = PlayerVars.RunCommandWithBash("redis-cli HGET user"+my_num+" right", true);
		PlayerVars.RunCommandWithBash("redis-cli HSET user"+my_num+" right 0", false);
		return Int32.Parse(result);
	}
	public int GetLeft() // get player's left input
	{
		var result = PlayerVars.RunCommandWithBash("redis-cli HGET user"+my_num+" left", true);
		PlayerVars.RunCommandWithBash("redis-cli HSET user"+my_num+" left 0", false);
		return Int32.Parse(result);
	}
}