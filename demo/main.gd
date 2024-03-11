extends Node3D

@onready var left_hand: XRController3D = $XROrigin3D/LeftHand
@onready var right_hand: XRController3D = $XROrigin3D/RightHand
@onready var left_hand_mesh: MeshInstance3D = $XROrigin3D/LeftHand/LeftHandMesh
@onready var right_hand_mesh: MeshInstance3D = $XROrigin3D/RightHand/RightHandMesh
@onready var left_controller_model: OpenXRFbRenderModel = $XROrigin3D/LeftHand/LeftControllerFbRenderModel
@onready var right_controller_model: OpenXRFbRenderModel = $XROrigin3D/RightHand/RightControllerFbRenderModel
@onready var floor_mesh: MeshInstance3D = $Floor
@onready var world_environment: WorldEnvironment = $WorldEnvironment
@onready var scene_manager: OpenXRFbSceneManager = $XROrigin3D/OpenXRFbSceneManager

var xr_interface : XRInterface = null
var hand_tracking_source: Array[OpenXRInterface.HandTrackedSource]
var passthrough_enabled: bool = false

# Called when the node enters the scene tree for the first time.
func _ready():
	xr_interface = XRServer.find_interface("OpenXR")
	if xr_interface and xr_interface.is_initialized():
		var vp: Viewport = get_viewport()
		vp.use_xr = true
		print("blend modes: ", xr_interface.get_supported_environment_blend_modes())

	hand_tracking_source.resize(OpenXRInterface.HAND_MAX)
	for hand in OpenXRInterface.HAND_MAX:
		hand_tracking_source[hand] = xr_interface.get_hand_tracking_source(hand)


func enable_passthrough(enable: bool) -> void:
	if passthrough_enabled == enable:
		return

	var supported_blend_modes = xr_interface.get_supported_environment_blend_modes()
	print ("Supported blend modes: ", supported_blend_modes)
	if XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND in supported_blend_modes and XRInterface.XR_ENV_BLEND_MODE_OPAQUE in supported_blend_modes:
		print ("Passthrough supported.")
		if enable:
			# Switch to passthrough.
			xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND
			get_viewport().transparent_bg = true
			world_environment.environment.background_mode = Environment.BG_COLOR
			world_environment.environment.background_color = Color(0.0, 0.0, 0.0, 0.0)
			floor_mesh.visible = false
			scene_manager.visible = true
			if not scene_manager.are_scene_anchors_created():
				scene_manager.create_scene_anchors()
		else:
			# Switch back to VR.
			xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_OPAQUE
			get_viewport().transparent_bg = false
			world_environment.environment.background_mode = Environment.BG_SKY
			floor_mesh.visible = true
			scene_manager.visible = false
		passthrough_enabled = enable
	else:
		print("Switching to/from passthrough not supported.")


func _physics_process(_delta: float) -> void:
	for hand in OpenXRInterface.HAND_MAX:
		var source = xr_interface.get_hand_tracking_source(hand)
		if hand_tracking_source[hand] == source:
			continue

		var controller = left_controller_model if (hand == OpenXRInterface.HAND_LEFT) else right_controller_model
		controller.visible = (source == OpenXRInterface.HAND_TRACKED_SOURCE_CONTROLLER)

		if source == OpenXRInterface.HAND_TRACKED_SOURCE_UNOBSTRUCTED:
			match hand:
				OpenXRInterface.HAND_LEFT:
					left_hand.tracker = "/user/fbhandaim/left"
				OpenXRInterface.HAND_RIGHT:
					right_hand.tracker = "/user/fbhandaim/right"
		else:
			match hand:
				OpenXRInterface.HAND_LEFT:
					left_hand.tracker = "left_hand"
					left_hand.pose = "grip"
				OpenXRInterface.HAND_RIGHT:
					right_hand.tracker = "right_hand"
					right_hand.pose = "grip"

		hand_tracking_source[hand] = source


func _on_left_hand_button_pressed(name):
	if name == "menu_button":
		print("Triggering scene capture")
		scene_manager.request_scene_capture()

	elif name == "by_button":
		enable_passthrough(not passthrough_enabled)

	elif name == "ax_button":
		print ("Spatial Entity Query is supported: ", OpenXRFbSpatialEntityQueryExtensionWrapper.is_spatial_entity_query_supported())
		var query = OpenXRFbSpatialEntityQuery.new()
		#query.query_by_uuid(["77530fc0-9595-ecf6-791c-019e882a8f48", "860f3503-06bc-75a9-e412-ec081b9d33d9"])
		#query.max_results = 20
		#query.query_by_component(OpenXRFbSpatialEntity.COMPONENT_TYPE_ROOM_LAYOUT)
		if query.execute() == OK:
			var results = await query.completed
			if results.size() == 0:
				print ("No results")
			for e in results:
				print (" ===== Entity: ", e.get_uuid(), " - ", e.get_semantic_labels(), " ===== ")
				var room_layout = e.get_room_layout()
				if room_layout.size() > 0:
					print ("Room layout: ", room_layout)
				var contained = e.get_contained_uuids()
				if contained.size() > 0:
					print ("Contained: ", contained)
				if e.is_component_enabled(OpenXRFbSpatialEntity.COMPONENT_TYPE_BOUNDED_2D):
					print ("Bounding Box 2D :", e.get_bounding_box_2d())
					print ("Boundary 2D :", e.get_boundary_2d())
				if e.is_component_enabled(OpenXRFbSpatialEntity.COMPONENT_TYPE_BOUNDED_3D):
					print ("Bounding Box 3D :", e.get_bounding_box_3d())
		else:
			print ("Error running spatial entity query")


func _on_left_controller_fb_render_model_render_model_loaded() -> void:
	left_hand_mesh.hide()


func _on_right_controller_fb_render_model_render_model_loaded() -> void:
	right_hand_mesh.hide()

func _on_scene_manager_scene_capture_completed(success: bool) -> void:
	print ("Scene Capture Complete: ", success)
	if success:
		# Recreate scene anchors since the user may have changed them.
		if scene_manager.are_scene_anchors_created():
			scene_manager.remove_scene_anchors()
			scene_manager.create_scene_anchors()

		# Switch to passthrough.
		enable_passthrough(true)

func _on_scene_manager_scene_data_missing() -> void:
	scene_manager.request_scene_capture()
