extends Area2D

# Andre Koka - The Living Board Game - Fall 2023
# script to control the shop
# involves calling serial-related functions from SerialManager.cs

# changeable to make new shops
export var attack_stat: int = 5 # attack of combat equipment
export var defense_stat: int = 3 # defense of combat equipment
export var move_stat: int = 3 # movement of overworld equipment
export var view_stat: int = 1 # view of overworld equipment
export var max_health: int = 50 # amount to increase max health
export var healing: int = 35 # amount of instant healing

# prices
export var combat_price: int = 100 
export var overworld_price: int = 150
export var max_health_price: int = 75


var player
var up_sold: bool = false
var right_sold: bool = false
var left_sold: bool = false
var delay: int = 0
var SerialManager

# Called when the node enters the scene tree for the first time.
func _ready(): # creating UI for the shop
	$ShopUI/CombatEquip/price.text = "$"+str(combat_price)
	$ShopUI/OverworldEquip/price.text = "$"+str(overworld_price)
	$ShopUI/HealthUp/price.text = "$"+str(max_health_price)
	
	$ShopUI/CombatEquip/attack.text = "+ " + str(attack_stat)
	$ShopUI/CombatEquip/defense.text = "+ " + str(defense_stat)
	$ShopUI/OverworldEquip/speed.text = "+ " + str(move_stat)
	$ShopUI/OverworldEquip/view.text = "+ " + str(view_stat)
	
	var string = "+" + str(healing)
	$ShopUI/HealthUp/heal.text = string + " Healing"
	string = "+" + str(max_health)
	$ShopUI/HealthUp/max_up.text = string + " Max Health"
	
	SerialManager = get_node("/root/Board/SerialManager")


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	if delay>0: # prevent accidental buying when entering
		delay -= 1
	
	if $ShopUI.visible == true and delay == 0:
		if player.GetRight() == 1: # buy overworld equipment
			if !right_sold and PlayerVars.play_gold[player.my_num-1] >= overworld_price:
				$ShopUI/OverworldEquip.visible = false
				$ShopUI/OverworldSold.visible = true
				PlayerVars.play_gold[player.my_num-1] -= overworld_price
				
				# send message to change actual gold value
				var temp = SerialManager.make_stats(PlayerVars.speed[player.my_num-1], PlayerVars.view[player.my_num-1], PlayerVars.health[player.my_num-1], PlayerVars.play_gold[player.my_num-1], PlayerVars.defense[player.my_num-1], PlayerVars.attack[player.my_num-1])
				SerialManager.send_player_msg(player.my_num, temp)
				
				# mark as sold and make new equipment
				right_sold = true
				SerialManager.make_equip(move_stat, view_stat, 0, 0, "   Overworld   It'll help you get around faster, I guess!")
		if player.GetLeft() == 1: # buy combat equipment
			if !left_sold and PlayerVars.play_gold[player.my_num-1] >= combat_price:
				$ShopUI/CombatEquip.visible = false
				$ShopUI/CombatSold.visible = true
				PlayerVars.play_gold[player.my_num-1] -= combat_price
				
				# send message to change actual gold value
				var temp = SerialManager.make_stats(PlayerVars.speed[player.my_num-1], PlayerVars.view[player.my_num-1], PlayerVars.health[player.my_num-1], PlayerVars.play_gold[player.my_num-1], PlayerVars.attack[player.my_num-1], PlayerVars.defense[player.my_num-1])
				SerialManager.send_player_msg(player.my_num, temp)
				
				# mark as sold and make new equipment
				left_sold = true
				SerialManager.make_equip(0, 0, attack_stat, defense_stat, "    Weapon!    Can help you out in a fight, if you know how to use it.")
		if player.GetUp() == 1: # buy instant healing + max health
			if !up_sold and PlayerVars.play_gold[player.my_num-1] >= max_health_price:
				$ShopUI/HealthUp.visible = false
				$ShopUI/HealthSold.visible = true
				PlayerVars.max_health[player.my_num-1] += max_health
				PlayerVars.health[player.my_num-1] += healing
				PlayerVars.play_gold[player.my_num-1] -= max_health_price
				
				# send message to change actual player health and gold
				var temp = SerialManager.make_stats(PlayerVars.speed[player.my_num-1], PlayerVars.view[player.my_num-1], PlayerVars.health[player.my_num-1], PlayerVars.play_gold[player.my_num-1], PlayerVars.attack[player.my_num-1], PlayerVars.defense[player.my_num-1])
				SerialManager.send_player_msg(player.my_num, temp)
				
				# mark as sold
				up_sold = true
		if player.GetDown() == 1: # leave the shop, deleting if it is sold out
			if right_sold and up_sold and left_sold:
				queue_free()
			else:
				$ShopUI.visible = false
			PlayerVars.reset_inputs()
			get_tree().paused = false # resume overworld

# when shop first entered by a player
func _on_Shop_area_entered(area):
	if area.is_in_group("Player"):
		get_tree().paused = true # pause overworld
		$ShopUI.visible = true
		delay = 50 # wait 50 frames before shop is active
		player = area
		PlayerVars.reset_inputs()

# when shop exited (uneccessary)
func _on_Shop_area_exited(area):
	if area.is_in_group("Player"):
		print("leaving shop")
