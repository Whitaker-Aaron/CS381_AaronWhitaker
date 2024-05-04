extends Control

@onready var game = $"../Game"
@onready var address_entry = $PanelContainer/VBoxContainer/IPAddress
@onready var label = $"../Game/Label"
@onready var hostText = $"../Game/Player1ID"
@onready var peerText = $"../Game/Player2ID"

var hostID = ""
var peerID = ""


#PORTs are 16 bit numbers. Specific location where our packets arrive. 
const PORT = 9999

#UDP is the unreliable one 
#TCP is the reliable one. 
#REPLICATE: 
#Replicate Always: Every frame 
#Replicate On Change: Event based 
#

#ENet is a library that builds reliability on top of the 
#unreliable networking stack. 
#
#
var peer = ENetMultiplayerPeer.new()

func buttonCommon():
	hide()
	game.show()
	
#Any function that starts with an underscore is private.
#Otherwise public
func _on_host_button_pressed():
	buttonCommon()
	
	
	
	peer.create_server(PORT)
	multiplayer.multiplayer_peer = peer
	
	#Event so when a peer connects, we call the addPlayer. 
	multiplayer.peer_connected.connect(addPlayer)
	multiplayer.peer_disconnected.connect(removePlayer)
	label.updateValue(0)
	
	hostID = "Host ID: " + String.num(multiplayer.get_unique_id())
	hostText.addText(hostID)
	
func _on_join_button_pressed():
	buttonCommon()
	
	peer.create_client(address_entry.text, PORT)
	multiplayer.multiplayer_peer = peer
	#print(address_entry.text)
	
	peerID = "Peer ID: " + String.num(multiplayer.get_unique_id())
	peerText.addText(peerID)
	
func addPlayer(peer_id):
	print(peer_id)
	
func removePlayer(peer_id):
	pass
	


	



## Called when the node enters the scene tree for the first time.
#func _ready():
	#pass # Replace with function body.
#
#
## Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
	#pass
