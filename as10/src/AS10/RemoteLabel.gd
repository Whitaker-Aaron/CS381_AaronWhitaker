extends Label

var second = 1
var counter = 0



@rpc("any_peer")
func updateValueRPC(newValue, senderID):
	if is_multiplayer_authority() or senderID == 0:
		#Gives every player connected to the host a unique id.
		#Host is ID 1. 
		updateValue(newValue, senderID)

func updateValue(newValue, senderID = 1):
	#Function gets recursively called but on the host if called from client.
	if !(is_multiplayer_authority() or senderID == 0):
		updateValueRPC.rpc_id(1, newValue, multiplayer.get_unique_id())
		return
		
	counter = newValue
	text = "Counter: " + String.num(counter)
	
	if is_multiplayer_authority() and senderID != 0:
		print(senderID, "has set the value to: ", counter)
		updateValueRPC.rpc(newValue, 0)
	#RPC : Remote Procedure Call 
	#Call another function from a different pc. 
	


# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if !is_multiplayer_authority(): return
	
	
	second -= delta
	if second < 0: 
		second = 1
		
		#text = "Counter: " + str(counter)
	pass
	
func _on_increment_button_pressed():
	updateValue(counter + 1)
	
func _on_decrement_button_pressed():
	updateValue(counter - 1)
