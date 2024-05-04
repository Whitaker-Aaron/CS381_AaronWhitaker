extends Label

#@onready var counterLabel = $"../Label"
@onready var network = $"../../HostMenu"
@onready var synch = $MultiplayerSynchronizer

var update = false


# Called when the node enters the scene tree for the first time.
func _ready():
	#text = "Peer ID: "
	#pass # Replace with function body.
	pass


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if(update):
		addText(text)
	else:
		#print(multiplayer.has_multiplayer_peer())
		if(!(multiplayer.get_peers().is_empty())):
			#print("null")
			text = "Peer ID: " + String.num(multiplayer.get_peers()[0])
	
		
		#print(multiplayer.get_peers())
		
		pass
		
	
func addText(peerID):
	text = peerID
	update = true
	
	
