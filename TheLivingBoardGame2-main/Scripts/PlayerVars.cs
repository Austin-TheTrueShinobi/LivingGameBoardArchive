using Godot;
using System;
using System.IO.Ports;
using System.Diagnostics;
using System.Runtime.InteropServices;

public partial class PlayerVars : Node
{
	// Andre Koka - The Living Board Game - Fall 2023
	// script that acts as a singleton - basically holds variables across scenes
	// extremely important script, as serial info is often stored here for other scenes

	// management variables
	public static SerialPort port; // store .NET serial port object
	public static int num_players = 0;
	public int cur_player = 0; // current player
	public static byte[,] uid = new byte[4, 7]; // the UID (7 bytes) of each player
	public static int[] player_num = {0, 0, 0, 0};
	public static int[] connected = {0, 0, 0, 0}; // 1 = true, 0 = false
	public static int[] on_main_board = {0, 0, 0, 0}; // 1 = true, 0 = false
	public static int[] counted = {0, 0, 0, 0}; // 1 = true, 0 = false
	public static int[] tracking = {0, 0, 0, 0}; // 1 = true, 0 = false
	public static bool isLinux; // true when running on Linux system

	// player stats
	public static byte[] p_class = {0, 0, 0, 0}; // max value 9
	public byte[] max_health = {100, 100, 100, 100}; // max value 127
	public static byte[] health = {50, 50, 50, 50}; // max value 127
	public static byte[] speed = {100, 50, 50, 50}; // max value 9
	public static byte[] attack = {5, 5, 5, 5}; // max value 127
	public static byte[] defense = {1, 1, 1, 1}; // max value 127
	public static byte[] view = {9, 9, 9, 9}; // max value 9
	public static int[] gold = {1000, 1000, 1000, 1000}; // max value 3000
	public int[] play_gold = {1000, 1000, 1000, 1000}; // non-static version of gold
	public ushort[] special = {0, 0, 0, 0}; // unused but still present in message
	

	Godot.Collections.Dictionary player_start = new Godot.Collections.Dictionary
	{
    	{1, new Vector2(16, 16)},   // top left corner
		{2, new Vector2(16, 448)},  // bottom left corner
		{3, new Vector2(624, 16)},  // top right corner
		{4, new Vector2(624, 448)}, // bottom right corner  
	};

	Godot.Collections.Dictionary player_color = new Godot.Collections.Dictionary
	{
    	{1, new Color(1, 0, 0)},   //player 1 - red
		{2, new Color(0, 1, 0)},   //player 2 - green
		{3, new Color(0, 0, 1)},   //player 3 - blue
		{4, new Color(1, 1, 0)},   //player 4 - yellow  
	};

	// custom sprite depending on player class
	Godot.Collections.Dictionary sprites = new Godot.Collections.Dictionary
	{
		{0, ResourceLoader.Load("res://Assets/Player.png")},
    	{1, ResourceLoader.Load("res://Assets/dog.png")},  
		{2, ResourceLoader.Load("res://Assets/Scientist.png")},   
		{3, ResourceLoader.Load("res://Assets/goblin.png")},   
		{4, ResourceLoader.Load("res://Assets/robot.png")}, 
		{5, ResourceLoader.Load("res://Assets/mutant.png")}, 
		{6, ResourceLoader.Load("res://Assets/scarecrow.png")}, 
	};

	// called once at program start
	public override void _Ready()
    {
		isLinux = System.Runtime.InteropServices.RuntimeInformation.IsOSPlatform(OSPlatform.Linux);
		if(isLinux)
		{
			// get serial port names for debugging purposes
			string[] ports = SerialPort.GetPortNames();
			GD.Print("The following serial ports were found:");
			foreach(string port in ports)
				GD.Print(port);
			
			port = new SerialPort(ports[0], 9600); // assuming only one port found, which usually is the case
			port.RtsEnable = true; // needed for pico
			port.DtrEnable = true; // needed for pico
			port.ReadTimeout = 1000; // arbitrary
			port.WriteTimeout = 1000; // arbitrary
			port.Open();
		}
    }

	// custom function for running bash commands, used to access Redis
	public static string RunCommandWithBash(string command, bool want_out)
	{
    	var psi = new ProcessStartInfo();
    	psi.FileName = "/bin/bash";
    	psi.Arguments = $"-c \"{command}\"";
 		psi.RedirectStandardOutput = true;
    	psi.UseShellExecute = false;
    	psi.CreateNoWindow = true;

    	var process = Process.Start(psi);
		string output = null;
		process.WaitForExit();
		if(want_out)
    		output = process.StandardOutput.ReadToEnd();
		process.Close();

    	return output;
	}

	// avoid input buffering from Redis
	public static void reset_inputs()
	{
		RunCommandWithBash("redis-cli HSET user1 left 0", false);
		RunCommandWithBash("redis-cli HSET user1 right 0", false);
		RunCommandWithBash("redis-cli HSET user1 up 0", false);
		RunCommandWithBash("redis-cli HSET user1 down 0", false);
		RunCommandWithBash("redis-cli HSET user2 left 0", false);
		RunCommandWithBash("redis-cli HSET user2 right 0", false);
		RunCommandWithBash("redis-cli HSET user2 up 0", false);
		RunCommandWithBash("redis-cli HSET user2 down 0", false);
		RunCommandWithBash("redis-cli HSET user3 left 0", false);
		RunCommandWithBash("redis-cli HSET user3 right 0", false);
		RunCommandWithBash("redis-cli HSET user3 up 0", false);
		RunCommandWithBash("redis-cli HSET user3 down 0", false);
		RunCommandWithBash("redis-cli HSET user4 left 0", false);
		RunCommandWithBash("redis-cli HSET user4 right 0", false);
		RunCommandWithBash("redis-cli HSET user4 up 0", false);
		RunCommandWithBash("redis-cli HSET user4 down 0", false);
	}

	// get AI facial recognition model result
	public string get_emotion()
	{
		if(isLinux)
			return RunCommandWithBash("redis-cli HGET emotions averageMood", true); // happy sad angry contempt scared neutral
		return "";
	}
}
