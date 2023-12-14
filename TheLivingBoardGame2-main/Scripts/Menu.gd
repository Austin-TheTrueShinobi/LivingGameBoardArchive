extends Node2D

# Andre Koka - The Living Board Game - Fall 2023
# small script to control the main menu

# Called when the node enters the scene tree for the first time.
func _ready():
	$StartGame.grab_focus()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	pass

# emitted when start game button pressed
func _on_StartGame_pressed():
	var next_scene = preload("res://Scenes/Board.tscn")
	get_tree().root.add_child(next_scene.instance())
	set_process(false);
	queue_free()

# emitted when help button pressed
func _on_Help_pressed():
	$HelpScreen/Back.visible = true
	$HelpScreen/Back.grab_focus()
	$HelpScreen/Background.visible = true
	$HelpScreen/HelpLabel.visible = true
	$HelpScreen/HelpLabel2.visible = true
	$HelpScreen/HelpLabel3.visible = true
	$HelpScreen/HelpLabel4.visible = true
	$HelpScreen/BackLabel.visible = true
	self.visible = false

# emitted when back button pressed
func _on_Back_pressed():
	$HelpScreen/Back.visible = false
	$HelpScreen/Background.visible = false
	$HelpScreen/HelpLabel.visible = false
	$HelpScreen/HelpLabel2.visible = false
	$HelpScreen/HelpLabel3.visible = false
	$HelpScreen/HelpLabel4.visible = false
	$HelpScreen/BackLabel.visible = false
	self.visible = true
	$StartGame.grab_focus()
