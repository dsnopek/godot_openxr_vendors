extends StartXR

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	get_tree().on_request_permissions_result.connect(_on_request_permissions_result)
	
	var openxr_interface = XRServer.find_interface("OpenXR")
	if not openxr_interface or not openxr_interface.is_initialized():
		print("OpenXR interface not found or not initialized")
		return

	get_viewport().use_xr = true

func _on_openxr_session_begun() -> void:
	super()
	start_light_estimation()


func start_light_estimation() -> void:
	if not OpenXRAndroidLightEstimationExtension.is_light_estimation_supported():
			push_error("Light estimation is unsupported")
			return

	if OpenXRAndroidLightEstimationExtension.start_light_estimation():
			print("Light estimation started")
	else:
			push_error("Unable to start light estimation")


func _on_request_permissions_result(p_permission: String, p_granted: bool) -> void:
	if p_permission == "android.permission.SCENE_UNDERSTANDING_COARSE" and p_granted:
			start_light_estimation()
