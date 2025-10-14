extends Node

@export var curve_2d: Curve

func _ready() -> void:
	#RenderingServer.set_debug_generate_wireframes(true)
	#get_viewport().debug_draw = Viewport.DEBUG_DRAW_WIREFRAME
	var cin := ChunkinatorTest.new()
	add_child(cin)
	$FlyCamera.get_camera().far = 15000
	
	var ast := AStarGrid2D.new()
	ast.region = Rect2i(-16, -16, 32, 32)
	ast.cell_size = Vector2i(2, 2)
	ast.update()
	print(ast.get_id_path(Vector2i(-4, 0), Vector2i(15, 15)))
func _process(delta: float) -> void:
	pass
