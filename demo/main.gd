extends Node3D

@onready var left_hand_mesh: MeshInstance3D = $XROrigin3D/LeftHand/LeftHandMesh
@onready var right_hand_mesh: MeshInstance3D = $XROrigin3D/RightHand/RightHandMesh
@onready var floor_mesh: MeshInstance3D = $Floor
@onready var world_environment: WorldEnvironment = $WorldEnvironment

var xr_interface : XRInterface = null

# Called when the node enters the scene tree for the first time.
func _ready():
	xr_interface = XRServer.find_interface("OpenXR")
	if xr_interface and xr_interface.is_initialized():
		var vp: Viewport = get_viewport()
		vp.use_xr = true
		print("blend modes: ", xr_interface.get_supported_environment_blend_modes())

	var scene_capture := Engine.get_singleton("OpenXRFbSceneCaptureExtensionWrapper")
	if scene_capture:
		scene_capture.connect("scene_capture_completed", _on_scene_capture_completed)



func _on_scene_capture_completed():
	print("Scene Capture completed")


func _on_left_hand_button_pressed(name):
	var scene_capture := Engine.get_singleton("OpenXRFbSceneCaptureExtensionWrapper")
	if name == "menu_button" and scene_capture:
		print("Triggering scene capture")
		scene_capture.request_scene_capture()

	elif name == "by_button":
		var supported_blend_modes = xr_interface.get_supported_environment_blend_modes()
		print ("Supported blend modes: ", supported_blend_modes)
		if XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND in supported_blend_modes and XRInterface.XR_ENV_BLEND_MODE_OPAQUE in supported_blend_modes:
			print ("Passthrough supported.")
			if xr_interface.environment_blend_mode == XRInterface.XR_ENV_BLEND_MODE_OPAQUE:
				# Switch to passthrough.
				xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND
				get_viewport().transparent_bg = true
				world_environment.environment.background_mode = Environment.BG_COLOR
				world_environment.environment.background_color = Color(0.0, 0.0, 0.0, 0.0)
				floor_mesh.visible = false
			elif xr_interface.environment_blend_mode == XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND:
				# Switch back to VR.
				xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_OPAQUE
				get_viewport().transparent_bg = false
				world_environment.environment.background_mode = Environment.BG_SKY
				floor_mesh.visible = true
		else:
			print("Switching to/from passthrough not supported.")

	elif name == "ax_button":
		print ("Spatial Entity Query is supported: ", OpenXRFbSpatialEntityQueryExtensionWrapper.is_spatial_entity_query_supported())
		var query = OpenXRFbSpatialEntityQuery.new()
		#query.query_by_uuid(["77530fc0-9595-ecf6-791c-019e882a8f48", "860f3503-06bc-75a9-e412-ec081b9d33d9"])
		#query.max_results = 20
		query.query_by_component(OpenXRFbSpatialEntityQuery.COMPONENT_TYPE_ROOM_LAYOUT)
		if query.execute() == OK:
			var results = await query.completed
			print ("Spatial Entity Query results: ", results)
		else:
			print ("Error running spatial entity query")


func _on_left_controller_fb_render_model_render_model_loaded() -> void:
	left_hand_mesh.hide()


func _on_right_controller_fb_render_model_render_model_loaded() -> void:
	right_hand_mesh.hide()

