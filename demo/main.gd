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

	#var ray_intersection: Vector2 = cylinder_intersects_ray($XROrigin3D/OpenXRCompositionLayerCylinder, right_hand.transform.origin, -$XROrigin3D/RightHandPointer.transform.basis.z)
	#print ("Ray intersection: ", ray_intersection)
	#$SubViewport/Control.update_pointer(ray_intersection)

	var ray_intersection: Vector2 = equirect_intersects_ray($XROrigin3D/OpenXRCompositionLayerEquirect, right_hand.transform.origin, -$XROrigin3D/RightHandPointer.transform.basis.z)
	print ("Ray intersection: ", ray_intersection)
	$SubViewport/Control.update_pointer(ray_intersection)

	#var ray_intersection: Vector2 = quad_intersects_ray($XROrigin3D/OpenXRCompositionLayerQuad, right_hand.transform.origin, -$XROrigin3D/RightHandPointer.transform.basis.z)
	#print ("Ray intersection: ", ray_intersection)
	#$SubViewport/Control.update_pointer(ray_intersection)


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

	var correction: Basis = p_layer.transform.basis.inverse()
	correction = correction.rotated(Vector3.UP, -PI / 2.0)
	var relative_point: Vector3 = correction * (intersection - cylinder_center)

	var projected_point: Vector2 = Vector2(relative_point.x, relative_point.z)
	var intersection_angle: float = atan2(projected_point.y, projected_point.x)
	if abs(intersection_angle) > cylinder_angle / 2.0:
		return Vector2(-1.0, -1.0)

	var arc_length = cylinder_radius * cylinder_angle
	var height = cylinder_aspect_ratio * arc_length
	if abs(relative_point.y) > height / 2.0:
		return Vector2(-1.0, -1.0)

	var u: float = 0.5 + (intersection_angle / cylinder_angle)
	var v: float = 1.0 - (0.5 + (relative_point.y / height))

	return Vector2(u, v)

func equirect_intersects_ray(p_layer: OpenXRCompositionLayerEquirect, p_origin: Vector3, p_direction: Vector3) -> Vector2:
	var equirect_radius: float = p_layer.radius
	var equirect_center: Vector3 = p_layer.position
	var equirect_central_horizontal_angle: float = p_layer.central_horizontal_angle
	var equirect_upper_vertical_angle: float = p_layer.upper_vertical_angle
	var equirect_lower_vertical_angle: float = p_layer.lower_vertical_angle

	var offset: Vector3 = p_origin - equirect_center
	var a: float = p_direction.dot(p_direction)
	var b: float = 2.0 * offset.dot(p_direction)
	var c: float = offset.dot(offset) - equirect_radius * equirect_radius

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

	var correction: Basis = p_layer.transform.basis.inverse()
	correction = correction.rotated(Vector3.UP, -PI / 2.0)
	var relative_point: Vector3 = correction * (intersection - equirect_center)

	var horizontal_intersection_angle = atan2(relative_point.z, relative_point.x)
	if abs(horizontal_intersection_angle) > equirect_central_horizontal_angle / 2.0:
		return Vector2(-1.0, -1.0)

	var vertical_intersection_angle = acos(relative_point.y / equirect_radius) - (PI / 2.0)
	print ("Vertical intersection angle: ", vertical_intersection_angle)
	if vertical_intersection_angle < 0:
		if abs(vertical_intersection_angle) > equirect_upper_vertical_angle:
			return Vector2(-1.0, -1.0)
	else:
		if vertical_intersection_angle > equirect_lower_vertical_angle:
			return Vector2(-1.0, -1.0)

	# Recenter the vertical angle if it's uneven.
	if equirect_upper_vertical_angle != equirect_lower_vertical_angle:
		vertical_intersection_angle -= (-equirect_upper_vertical_angle + equirect_lower_vertical_angle) / 2.0

	var u: float = 0.5 + (horizontal_intersection_angle / equirect_central_horizontal_angle)
	var v: float = 0.5 + (vertical_intersection_angle / (equirect_upper_vertical_angle + equirect_lower_vertical_angle))

	return Vector2(u, v)

func quad_intersects_ray(p_layer: OpenXRCompositionLayerQuad, p_origin: Vector3, p_direction: Vector3) -> Vector2:
	var quad_center = p_layer.position
	var quad_normal = p_layer.transform.basis.z
	var quad_width = p_layer.quad_size.x
	var quad_height = p_layer.quad_size.y

	var denom = quad_normal.dot(p_direction)
	if abs(denom) > 0.0001:
		var vector: Vector3 = quad_center - p_origin
		var t = vector.dot(quad_normal) / denom
		if t < 0.0:
			return Vector2(-1.0, -1.0)
		var intersection: Vector3 = p_origin + p_direction * t
		$IntersectionSphere.position = intersection

		var relative_point: Vector3 = intersection - quad_center
		var projected_point: Vector2 = Vector2(
			relative_point.dot(p_layer.transform.basis.x),
			relative_point.dot(p_layer.transform.basis.y))
		if abs(projected_point.x) > quad_width / 2.0:
			return Vector2(-1.0, -1.0)
		if abs(projected_point.y) > quad_height / 2.0:
			return Vector2(-1.0, -1.0)

		var u: float = 0.5 + (projected_point.x / quad_width)
		var v: float = 1.0 - (0.5 + (projected_point.y / quad_height))

		return Vector2(u, v)

	return Vector2(-1.0, -1.0)

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
