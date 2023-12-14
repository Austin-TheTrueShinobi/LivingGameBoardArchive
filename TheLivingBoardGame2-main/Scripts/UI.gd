extends Node

# Andre Koka - The Living Board Game - Fall 2023
# script to control overworld UI
# also implements the polling of Redis for emotion data
# 	using PlayerVars.cs

var player
var gold
var delay: int = 0 # used to avoid constantly polling Redis
var emotion


# Called when the node enters the scene tree for the first time.
func _ready():
	pass

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	player = PlayerVars.cur_player
	$PlayerName.text = "Player "+ str(player)
	$Header.modulate = PlayerVars.player_color[player]
	
	var max_h = " / " + str(PlayerVars.max_health[player-1])
	var health = "Health: "+ str(PlayerVars.health[player-1])
	$Health.text = health + max_h
	
	$Moves.text = "Moves: "+ str(self.get_parent().get_parent().cur_player.moves_left)
	$Attack.text = str(PlayerVars.attack[player-1])
	$Defense.text = str(PlayerVars.defense[player-1])
	$View.text = str(PlayerVars.view[player-1])
	$Gold.text = str(PlayerVars.play_gold[player-1])
	if delay <= 0:
		# done to improve performance
		emotion = PlayerVars.get_emotion()
		delay = 100
	else:
		delay -= 1
	$Emotion.text = emotion
	
	# show text when player is not on the main board
	if PlayerVars.on_main_board[player-1] != 1:
		$OnBoard.visible = true
	else:
		$OnBoard.visible = false
	
# used to set gold from a static function in c#
func set_gold(var i, var g):
	print("setting gold")
	PlayerVars.play_gold[i] = g
