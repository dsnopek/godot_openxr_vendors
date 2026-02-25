extends Node3D


func _ready() -> void:
	if not OpenXRAndroidGoogleCloudAuthExtension.is_enabled():
		print("Google Cloud Authentication Extension is not supported on this device.")
		return

	var google_cloud_api_key := ""

	if FileAccess.file_exists("res://google_cloud_api_key.txt"):
		var file := FileAccess.open("res://google_cloud_api_key.txt", FileAccess.READ)
		if file:
			google_cloud_api_key = file.get_as_text().strip_edges()

	if google_cloud_api_key == "":
		print("Please set your Google Cloud API key in google_cloud_api_key.txt")
		return

	var result := OpenXRAndroidGoogleCloudAuthExtension.set_google_cloud_auth_api_key(google_cloud_api_key)
	if result:
		await result.completed
		if result.get_result_value():
			print("Google Cloud API key set successfully.")
		else:
			print("Failed to set Google Cloud API key.")
			return

	OpenXRAndroidGeospatialExtension.openxr_android_geospatial_state_changed.connect(_on_geospatial_state_changed)

	OpenXRAndroidGeospatialExtension.start_geospatial()


func _on_geospatial_state_changed(p_state) -> void:
	print("Geospatial state changed: ", p_state)
