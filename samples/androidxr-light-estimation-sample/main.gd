extends Node3D

var openxr_interface: OpenXRInterface

@onready var directional_light: DirectionalLight3D = $DirectionalLight3D
@onready var directional_light_orig_transform = directional_light.transform
@onready var directional_light_orig_color = directional_light.light_color
@onready var directional_light_orig_energy = directional_light.light_energy

@onready var world_environment: WorldEnvironment = $WorldEnvironment
@onready var environment: Environment = world_environment.environment
@onready var ambient_light_orig_source = environment.ambient_light_source
@onready var ambient_light_orig_color = environment.ambient_light_color
@onready var ambient_light_orig_energy = environment.ambient_light_energy


func _ready() -> void:
	get_tree().on_request_permissions_result.connect(_on_request_permissions_result)

	OS.request_permissions()

	openxr_interface = XRServer.find_interface("OpenXR")
	openxr_interface.session_begun.connect(_on_openxr_session_begun)

	OpenXRAndroidLightEstimationExtensionWrapper.light_estimate_types = OpenXRAndroidLightEstimationExtensionWrapper.LIGHT_ESTIMATE_TYPE_ALL

	var menu: Control = %Viewport2Din3D.get_scene_root()
	menu.mode_changed.connect(_on_mode_changed)
	menu.directional_light_mode_changed.connect(_on_directional_light_mode_changed)
	menu.ambient_light_mode_changed.connect(_on_ambient_light_mode_changed)
	menu.spherical_harmonics_degree_changed.connect(_on_spherical_harmonics_degree_changed)


func _on_openxr_session_begun() -> void:
	start_light_estimation()


func start_light_estimation() -> void:
	if OpenXRAndroidLightEstimationExtensionWrapper.start_light_estimation():
		print("Light estimation started")
	else:
		print("Unable to start light estimation")


func _on_request_permissions_result(p_permission: String, p_granted: bool) -> void:
	if p_permission == 'android.permission.SCENE_UNDERSTANDING_COARSE' and p_granted:
		start_light_estimation()


func _on_mode_changed(p_mode: int) -> void:
	pass


func _on_directional_light_mode_changed(p_mode: int) -> void:
	# Reset the original values.
	directional_light.transform = directional_light_orig_transform
	directional_light.light_color = directional_light_orig_color
	directional_light.light_energy = directional_light_orig_energy

	# Then change the mode.
	$OpenXRAndroidLightEstimation.directional_light_mode = p_mode


func _on_ambient_light_mode_changed(p_mode: int) -> void:
	# Reset the original values.
	environment.ambient_light_source = ambient_light_orig_source
	environment.ambient_light_color = ambient_light_orig_color
	environment.ambient_light_energy = ambient_light_orig_energy

	# Then change the mode.
	$OpenXRAndroidLightEstimation.ambient_light_mode = p_mode


func _on_spherical_harmonics_degree_changed(p_degree: int) -> void:
	$OpenXRAndroidLightEstimation.spherical_harmonics_degree = p_degree


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
