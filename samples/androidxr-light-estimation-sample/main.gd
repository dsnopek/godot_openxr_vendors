extends Node3D

var openxr_interface: OpenXRInterface

func _ready() -> void:
	get_tree().on_request_permissions_result.connect(_on_request_permissions_result)

	OS.request_permissions()

	openxr_interface = XRServer.find_interface("OpenXR")
	openxr_interface.session_begun.connect(_on_openxr_session_begun)

	OpenXRAndroidLightEstimationExtensionWrapper.light_estimate_types = OpenXRAndroidLightEstimationExtensionWrapper.LIGHT_ESTIMATE_TYPE_ALL

#	var sh := PackedVector3Array()
#	sh.resize(9)
#
#	var test_color := Color(0.2, 0.4, 0.9) / 0.2820947918
#	sh[0] = Vector3(test_color.r, test_color.g, test_color.b)
#	$WorldEnvironment.environment.sky.sky_material.set_shader_parameter("coefficients", sh)


func _on_openxr_session_begun() -> void:
	start_light_estimation()


func start_light_estimation() -> void:
	if OpenXRAndroidLightEstimationExtensionWrapper.start_light_estimation():
		print("Light estimation started")
	else:
		print("Unable to start light estimation")


func _on_request_permissions_result(p_permission: String, p_granted: bool) -> void:
	if p_permission == 'android.permission.SCENE_UNDERSTANDING_COARSE':
		start_light_estimation()


func _process(_delta: float) -> void:
	var directional_light_color = %DirectionalLight3D.light_color
	for child in %DirectionalLightIndicator.get_children():
		var material: StandardMaterial3D = child.get_surface_override_material(0)
		material.albedo_color = directional_light_color


func _on_timer_timeout() -> void:

	var ale = OpenXRAndroidLightEstimationExtensionWrapper
	if ale.is_light_estimation_started():
		print("---------------------------------------------------------------------------")
		print("is_estimate_valid: ", ale.is_estimate_valid())
		print("get_last_updated_time: ", ale.get_last_updated_time())
		print("is_directional_light_valid: ", ale.is_directional_light_valid())
		print("get_directional_light_intensity: ", ale.get_directional_light_intensity())
		print("get_directional_light_direction: ", ale.get_directional_light_direction())
		print("is_ambient_light_valid: ", ale.is_ambient_light_valid())
		print("get_ambient_light_intensity: ", ale.get_ambient_light_intensity())
		print("get_ambient_light_color_correction: ", ale.get_ambient_light_color_correction())
		print("is_spherical_harmonics_ambient_valid: ", ale.is_spherical_harmonics_ambient_valid())
		print("get_spherical_harmonics_ambient_coefficients: ", ale.get_spherical_harmonics_ambient_coefficients())
		print("is_spherical_harmonics_total_valid: ", ale.is_spherical_harmonics_total_valid())
		print("get_spherical_harmonics_total_coefficients: ", ale.get_spherical_harmonics_total_coefficients())

		#if ale.is_spherical_harmonics_ambient_valid():
		#	$WorldEnvironment.environment.sky.sky_material.set_shader_parameter("coefficients", ale.get_spherical_harmonics_ambient_coefficients())
		#	$WorldEnvironment.environment.sky.sky_material.set_shader_parameter("rotation", XRServer.world_origin.basis)
		#if ale.is_spherical_harmonics_total_valid():
		#	$WorldEnvironment.environment.sky.sky_material.set_shader_parameter("coefficients", ale.get_spherical_harmonics_total_coefficients())
		#	$WorldEnvironment.environment.sky.sky_material.set_shader_parameter("rotation", XRServer.world_origin.basis)
