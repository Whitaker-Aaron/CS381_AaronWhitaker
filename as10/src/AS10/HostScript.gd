extends Label

#@onready var counterLabel = $"../Label"
@onready var network = $"../../HostMenu"



# Called when the node enters the scene tree for the first time.
func _ready():
	text = "Host ID: " # Replace with function body.
	

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	#print(get_tree().get_network_unique_id())
	pass
	
	
func addText(hostID):
	text = hostID
	
