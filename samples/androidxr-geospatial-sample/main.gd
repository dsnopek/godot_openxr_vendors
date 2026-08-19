extends Node3D

@onready var camera: XRCamera3D = %XRCamera3D
@onready var timer: Timer = %Timer

const GEOSPATIAL_STATES = {
	OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_INITIALIZATION_FAILED: "Initialization Failed",
	OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_RUNNING: "Running",
	OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_STOPPED: "Stopped",
}

func _ready() -> void:
	var authenticated := await authenticate_with_google_cloud()
	print("Authenticated: ", authenticated)
	if not authenticated:
		return

	OpenXRAndroidGeospatialExtension.openxr_android_geospatial_state_changed.connect(_on_geospatial_state_changed)
	OpenXRAndroidGeospatialExtension.start_geospatial()


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
	var pose: OpenXRAndroidGeospatialPose = OpenXRAndroidGeospatialExtension.transform_to_geospatial_pose(camera.transform)
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
