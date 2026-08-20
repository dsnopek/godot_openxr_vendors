extends Node3D

const HUD = preload("res://hud.gd")

@onready var viewport_2d_in_3d = %Viewport2Din3D
@onready var camera: XRCamera3D = %XRCamera3D
@onready var timer: Timer = %Timer

const GEOSPATIAL_STATES = {
	OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_INITIALIZATION_FAILED: "Initialization Failed",
	OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_RUNNING: "Running",
	OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_STOPPED: "Stopped",
}

class LocationFinder:
	var _client
	var _task

	var _latitude: float
	var _longitude: float

	var _is_completed := false
	var _tried_fresh := false
	var _is_success := false

	signal completed(success: bool, latitude: float, longitude: float)

	func _init() -> void:
		_start()

	func is_completed() -> bool:
		return _is_completed

	func is_success() -> bool:
		return _is_success

	func get_latitude() -> float:
		return _latitude

	func get_longitude() -> float:
		return _longitude

	func _start() -> void:
		var android_runtime = Engine.get_singleton("AndroidRuntime")
		var activity = android_runtime.getActivity()

		var LocationServices = JavaClassWrapper.wrap("com.google.android.gms.location.LocationServices")

		_client = LocationServices.getFusedLocationProviderClient(activity)
		# Attempt to get the cached location.
		_task = _client.getLastLocation()

	func poll() -> void:
		if _is_completed:
			return
		if _task == null:
			return
		if not _task.isComplete():
			return

		if _task.isSuccessful():
			var location = _task.getResult()
			if location != null:
				_latitude = location.getLatitude()
				_longitude = location.getLongitude()
				_is_completed = true
				_is_success = true
				completed.emit(_is_success, _latitude, _longitude)

		# If we failed to get the cached location, we'll try fresh.
		if not _tried_fresh:
			_tried_fresh = true
			var Priority = JavaClassWrapper.wrap("com.google.android.gms.location.Priority")
			_task = _client.getCurrentLocation(Priority.PRIORITY_BALANCED_POWER_ACCURACY, null)
			return

		# If we failed both, then we're done.
		_task = null
		_is_completed = true
		completed.emit(false, 0.0, 0.0)


var location_finder: LocationFinder

func _ready() -> void:
	var hud: HUD = viewport_2d_in_3d.get_scene_root()

	var geospatial_supported := OpenXRAndroidGeospatialExtension.is_geospatial_supported()
	hud.geospatial_supported_field.text = str(geospatial_supported)
	if not geospatial_supported:
		print("Geospatial isn't supported.")
		return

	var authenticated := await authenticate_with_google_cloud()
	hud.authenticated_field.text = str(authenticated)
	print("Authenticated: ", authenticated)
	if not authenticated:
		return

	# If we want to confirm VPS availability before starting Geospatial,
	# then we need to get the location usual Android way.
	location_finder = LocationFinder.new()
	await location_finder.completed
	if not location_finder.is_success():
		print("Location not found.")
		return

	print("Location: ", location_finder.get_latitude(), ", ", location_finder.get_longitude())

	# Check VPS availability
	var vps_result := OpenXRAndroidGeospatialExtension.check_vps_availability(location_finder.get_latitude(), location_finder.get_longitude())
	await vps_result.completed
	if vps_result.get_status() != OpenXRFutureResult.RESULT_FINISHED:
		print("Getting VPS didn't finish: ", vps_result.get_status())

	var vps_available: bool = vps_result.get_result_value()
	hud.vps_availability_field.text = str(vps_available)
	print("VPS available: ", vps_available)
	if not vps_available:
		return

	OpenXRAndroidGeospatialExtension.openxr_android_geospatial_state_changed.connect(_on_geospatial_state_changed)
	OpenXRAndroidGeospatialExtension.start_geospatial()


func _process(_delta: float) -> void:
	if location_finder:
		location_finder.poll()


func authenticate_with_google_cloud() -> bool:
	if not OpenXRAndroidGoogleCloudAuthExtension.is_enabled():
		print("Google Cloud Authentication Extension is not supported on this device.")
		return false

	var google_cloud_api_key := ""

	if FileAccess.file_exists("res://google_cloud_api_key.txt"):
		var file := FileAccess.open("res://google_cloud_api_key.txt", FileAccess.READ)
		if file:
			google_cloud_api_key = file.get_as_text().strip_edges()

	if google_cloud_api_key == "":
		print("Please set your Google Cloud API key in google_cloud_api_key.txt")
		return false

	var result := OpenXRAndroidGoogleCloudAuthExtension.set_google_cloud_auth_api_key(google_cloud_api_key)
	if result:
		await result.completed
		if result.get_result_value():
			print("Google Cloud API key set successfully.")
		else:
			print("Failed to set Google Cloud API key.")
			return false

	return true


func _on_geospatial_state_changed(p_state) -> void:
	print("Geospatial state changed: ", GEOSPATIAL_STATES[p_state])

	if p_state == OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_RUNNING:
		# Start printing out pose information about the camera.
		timer.start()
	elif p_state == OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_STOPPED:
		timer.stop()


func _on_timer_timeout() -> void:
	var hud: HUD = viewport_2d_in_3d.get_scene_root()
	var pose: OpenXRAndroidGeospatialPose = OpenXRAndroidGeospatialExtension.transform_to_geospatial_pose(camera.transform)

	var position_valid := pose.is_position_valid()
	hud.position_valid_field.text = str(position_valid)
	if position_valid:
		hud.latitude_field.text = str(pose.latitude)
		hud.longitude_field.text = str(pose.longitude)
		hud.horizontal_accuracy_field.text = str(pose.horizontal_accuracy)
		hud.vertical_accuracy_field.text = str(pose.vertical_accuracy)

	var orientation_valid := pose.is_orientation_valid()
	hud.orientation_valid_field.text = str(orientation_valid)
	if orientation_valid:
		hud.orientation_field.text = str(pose.orientation)
		hud.orientation_yaw_accuracy_field.text = str(pose.orientation_yaw_accuracy)

	print("Position valid: ", pose.is_position_valid())
	print("Orientation valid: ", pose.is_orientation_valid())
	print("-----")
	print("Horizontal accuracy: ", pose.horizontal_accuracy)
	print("Vertical accuracy: ", pose.vertical_accuracy)
	print("Orientation/yaw accuracy: ", pose.orientation_yaw_accuracy)
	print("-----")
	print("Latitude: ", pose.latitude)
	print("Longitude: ", pose.longitude)
	print("Altitude: ", pose.altitude)
	print("Orientation: ", pose.orientation)
	print("")
