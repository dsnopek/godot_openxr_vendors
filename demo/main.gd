extends Node3D

@onready var left_hand: XRController3D = $XROrigin3D/LeftHand
@onready var right_hand: XRController3D = $XROrigin3D/RightHand
@onready var left_hand_mesh: MeshInstance3D = $XROrigin3D/LeftHand/LeftHandMesh
@onready var right_hand_mesh: MeshInstance3D = $XROrigin3D/RightHand/RightHandMesh
@onready var left_controller_model: OpenXRFbRenderModel = $XROrigin3D/LeftHand/LeftControllerFbRenderModel
@onready var right_controller_model: OpenXRFbRenderModel = $XROrigin3D/RightHand/RightControllerFbRenderModel

var xr_interface : XRInterface = null
var hand_tracking_source: Array[OpenXRInterface.HandTrackedSource]

# Called when the node enters the scene tree for the first time.
func _ready():
	xr_interface = XRServer.find_interface("OpenXR")
	if xr_interface and xr_interface.is_initialized():
		var vp: Viewport = get_viewport()
		vp.use_xr = true

	hand_tracking_source.resize(OpenXRInterface.HAND_MAX)
	for hand in OpenXRInterface.HAND_MAX:
		hand_tracking_source[hand] = xr_interface.get_hand_tracking_source(hand)

	var scene_capture := Engine.get_singleton("OpenXRFbSceneCaptureExtensionWrapper")
	if scene_capture:
		scene_capture.connect("scene_capture_completed", _on_scene_capture_completed)


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

	var ray_intersection: Vector2 = cylinder_intersects_ray($XROrigin3D/OpenXRCompositionLayerCylinder, right_hand.transform.origin, -$XROrigin3D/RightHandPointer.transform.basis.z)
	#print ("Ray intersection: ", ray_intersection)
	$SubViewport/Control.update_pointer(ray_intersection)

func cylinder_intersects_ray(p_layer: OpenXRCompositionLayerCylinder, p_origin: Vector3, p_direction: Vector3) -> Vector2:
	var cylinder_radius: float = p_layer.radius
	var cylinder_angle: float = p_layer.central_angle
	var cylinder_center: Vector3 = p_layer.position
	var cylinder_axis: Vector3 = p_layer.transform.basis.y
	var cylinder_aspect_ratio: float = p_layer.aspect_ratio

	var offset: Vector3 = p_origin - cylinder_center
	var a: float = p_direction.dot(p_direction - cylinder_axis * p_direction.dot(cylinder_axis))
	var b: float = 2.0 * (p_direction.dot(offset - cylinder_axis * offset.dot(cylinder_axis)))
	var c: float = offset.dot(offset - cylinder_axis * offset.dot(cylinder_axis)) - (cylinder_radius * cylinder_radius)

	var discriminant: float = b * b - 4.0 * a * c
	if discriminant < 0.0:
		return Vector2(-1.0, -1.0)

	var t0: float = (-b - sqrt(discriminant)) / (2.0 * a)
	var t1: float = (-b + sqrt(discriminant)) / (2.0 * a)
	var t: float = max(t0, t1)

	if t < 0.0:
		return Vector2(-1.0, -1.0)

	var intersection: Vector3 = p_origin + p_direction * t
	$IntersectionSphere.position = intersection
	var relative_point: Vector3 = p_layer.transform.basis.inverse() * (intersection - cylinder_center)
	var projected_point: Vector2 = Vector2(relative_point.x, relative_point.z)
	var intersection_angle: float = (PI / 2.0) + atan2(projected_point.y, projected_point.x)
	if abs(intersection_angle) > cylinder_angle / 2.0:
		return Vector2(-1.0, -1.0)

	var arc_length = cylinder_radius * cylinder_angle
	var height = cylinder_aspect_ratio * arc_length
	if abs(relative_point.y) > height / 2.0:
		return Vector2(-1.0, -1.0)

	var u: float = 0.5 + (intersection_angle / cylinder_angle)
	var v: float = 1.0 - (0.5 + (relative_point.y / height))

	return Vector2(u, v)

func _on_scene_capture_completed():
	print("Scene Capture completed")


func _on_left_hand_button_pressed(name):
	var scene_capture := Engine.get_singleton("OpenXRFbSceneCaptureExtensionWrapper")
	if name == "menu_button" and scene_capture:
		print("Triggering scene capture")
		scene_capture.request_scene_capture()

	if name == "ax_button":
		$OpenXRCompositionLayerQuad.visible = not $OpenXRCompositionLayerQuad.visible


func _on_left_controller_fb_render_model_render_model_loaded() -> void:
	left_hand_mesh.hide()


func _on_right_controller_fb_render_model_render_model_loaded() -> void:
	right_hand_mesh.hide()
