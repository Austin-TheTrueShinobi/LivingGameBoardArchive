extends Node2D

# Andre Koka - The Living Board Game - Fall 2023
# script for controller the overworld where the game takes place
# - adds all players into the board scene
# - assigns sprites, starting positions, colors to players
# - updates/controls camera
# - switches between turns of different players



var cur_player # easy access to current player node
var player_array = [] # array of players that are in-game
var camera_update = true # control when camera tracks the player
onready var camera = get_node("Camera2D")
export var camera_zoomed = false


# Called when the node enters the scene tree for the first time
func _ready():
	switch_player(false)
	
	# generate random start positions
	var rand_arr = [1, 2, 3, 4]
	randomize()
	rand_arr.shuffle()
	
	# add all players
	var new_player = preload("res://Scenes/Player.tscn")
	for n in 4:
		if(PlayerVars.connected[n] == 1):
			player_array.append(n+1)
		PlayerVars.play_gold[n] = PlayerVars.gold[n]
	
	for n in player_array:
		var new_instance = new_player.instance()
		new_instance.my_num = n
		var format_name = "Player%s"
		new_instance.name = format_name % str(n)
		new_instance.position = PlayerVars.player_start[rand_arr[n-1]]
		new_instance.moves_left = PlayerVars.speed[n-1]
		new_instance.get_node("Sprite").texture = PlayerVars.sprites[PlayerVars.p_class[n-1]]
		if(PlayerVars.cur_player == n):
			new_instance.inputs_enabled = true
			cur_player = new_instance
			new_instance.is_my_turn = 1
		else:
			new_instance.inputs_enabled = false
		add_child(new_instance)

# add keyboard ability to zoom camera and exit game
func _input(event):
	if cur_player.inputs_enabled:
		if event.is_action_pressed("ui_select"):
			next_turn()
		if event.is_action_pressed("ui_cancel"):
			get_tree().quit()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	if camera_update:
		if camera_zoomed:
			camera.position = Vector2(cur_player.position.x + 30, cur_player.position.y)
		else:
			camera.position = Vector2(400, 240)
	if cur_player.moves_left <= 0 and cur_player.is_moving == false:
		next_turn()

func next_turn(): # change to another players turn
	cur_player.inputs_enabled = false
	switch_player(true)
	PlayerVars.reset_inputs()
	cur_player.inputs_enabled = true
	
# update variables for new player (also used in initialization)
func switch_player(var first): 
	PlayerVars.cur_player += 1
	if PlayerVars.cur_player == 5:
		PlayerVars.cur_player = 1
	while PlayerVars.connected[PlayerVars.cur_player-1] == 0:
		PlayerVars.cur_player += 1
		if PlayerVars.cur_player == 5:
			PlayerVars.cur_player = 1
	if first:
		cur_player.is_my_turn = 0
		cur_player = get_node("Player%s" % str(PlayerVars.cur_player))	
		cur_player.moves_left = PlayerVars.speed[cur_player.my_num-1]
		cur_player.is_my_turn = 1
	
func tween_camera():
	if(camera_zoomed): # zoom out over time
		$Tween.interpolate_property($Camera2D, "position", $Camera2D.position, Vector2(400,240), 1, Tween.TRANS_LINEAR, Tween.EASE_IN_OUT)
		$Tween.interpolate_property($Camera2D, "zoom", $Camera2D.zoom, Vector2(1,1), 1, Tween.TRANS_LINEAR, Tween.EASE_IN_OUT)
	else: # zoom in over time
		$Tween.interpolate_property($Camera2D, "position", Vector2(400,240), Vector2(cur_player.position.x+30, cur_player.position.y), 1, Tween.TRANS_LINEAR, Tween.EASE_IN_OUT)
		$Tween.interpolate_property($Camera2D, "zoom", Vector2(1,1), Vector2(0.3,0.3), 1, Tween.TRANS_LINEAR, Tween.EASE_IN_OUT)
	camera_update = false
	$Tween.start()

# emitted when tween finishes
func _on_Tween_tween_all_completed():
	camera_update = true
	cur_player.inputs_enabled = true

# emitted when tween completes
func _on_Tween_tween_started(_object, _key):
	cur_player.inputs_enabled = false

# allow touchscreen button for zooming camera
func _on_TouchScreenButton_released():
	tween_camera()
	camera_zoomed = !camera_zoomed
