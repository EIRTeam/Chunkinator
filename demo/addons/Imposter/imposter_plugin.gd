@tool
extends EditorPlugin

var imposter_button: Button
var current_selection: Array[Node]
var imposter_window: Window
var single_bake:SingleBake

func _enter_tree():
	# Initialize when plugin enters scene tree
	single_bake=load("res://addons/Imposter/imposter/scenes/single_bake.tscn").instantiate() as SingleBake
	_setup_3d_toolbar()
	# Connect selection change signal
	EditorInterface.get_selection().selection_changed.connect(_on_selection_changed)


func _exit_tree():
	_cleanup_3d_toolbar()
	# Disconnect selection change signal
	if EditorInterface.get_selection().selection_changed.is_connected(_on_selection_changed):
		EditorInterface.get_selection().selection_changed.disconnect(_on_selection_changed)


## Setup 3D editor toolbar
func _setup_3d_toolbar():
	# Create imposter button
	imposter_button = Button.new()
	imposter_button.text = "Imposter"
	imposter_button.visible = false  # Initially hidden
	
	# Set button style to match editor native buttons
	_apply_editor_button_style(imposter_button)
	
	# Connect button click signal
	imposter_button.pressed.connect(_on_imposter_button_pressed)
	
	# Add button to 3D editor toolbar
	add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, imposter_button)
	
## Cleanup 3D editor toolbar
func _cleanup_3d_toolbar():
	if imposter_button and is_instance_valid(imposter_button):
		remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, imposter_button)
		imposter_button.queue_free()
		imposter_button = null
	
	# Cleanup window
	if imposter_window and is_instance_valid(imposter_window):
		imposter_window.queue_free()
		imposter_window = null
	
	if single_bake and is_instance_valid(single_bake):
		single_bake.queue_free()
		single_bake = null
	
## Called when selection changes
func _on_selection_changed():
	current_selection = EditorInterface.get_selection().get_selected_nodes()
	_update_imposter_button_visibility()
	
## Update imposter button visibility
func _update_imposter_button_visibility():
	if not imposter_button:
		return
	
	imposter_button.visible = current_selection.size()==1 and _has_geometry_instance_3d(current_selection[0])
	
## Check if node has GeometryInstance3D or its subclass
func _has_geometry_instance_3d(node: Node) -> bool:
	if not node or node is not Node3D:
		return false
	
	if node is GeometryInstance3D:
		return true
	
	for c in node.get_children():
		var res = _has_geometry_instance_3d(c)
		if res:
			return res
	
	# Check if node directly or indirectly inherits from GeometryInstance3D
	return false
	
## Called when imposter button is clicked
func _on_imposter_button_pressed():
	if current_selection.is_empty():
		return
	
	# Create and show Imposter window
	_show_imposter_window()
	
## Show Imposter window
func _show_imposter_window():
	# If window already exists, show directly
	if imposter_window and is_instance_valid(imposter_window):
		if imposter_window.visible:
			imposter_window.hide()
			return
		else:
			imposter_window.popup_centered(Vector2i(900, 600))
	else:
		# Create new window
		imposter_window = Window.new()
		imposter_window.add_child(single_bake)
		# Add to scene tree and show
		get_tree().root.add_child(imposter_window)
		
		imposter_window.title = "Imposter"
		imposter_window.size = Vector2i(900, 600)
		imposter_window.unresizable = false
		# Set window close behavior
		imposter_window.close_requested.connect(_close_imposter_window)
		imposter_window.wrap_controls = true

		single_bake.focus_mode = Control.FOCUS_ALL

		single_bake.grab_focus()
		imposter_window.popup_centered()
		
		# Set window as modal to ensure proper input handling
		imposter_window.transient = true
		imposter_window.exclusive = false
		
	single_bake.set_actor(current_selection[0])
	
	
## Unified window close handling function
func _close_imposter_window():
	if imposter_window and is_instance_valid(imposter_window):
		# Hide and destroy window
		imposter_window.hide()


## Apply editor button style
func _apply_editor_button_style(button: Button):
	# Set button to flat style, consistent with editor toolbar buttons
	button.flat = true
	
	## Get editor theme
	var editor_theme = EditorInterface.get_editor_theme()
	if editor_theme:
		# Apply editor theme
		button.theme = editor_theme
	
	# Set button minimum size and margins, consistent with other toolbar buttons
	button.custom_minimum_size = Vector2(0, 24)
	button.add_theme_constant_override("h_separation", 4)
	
