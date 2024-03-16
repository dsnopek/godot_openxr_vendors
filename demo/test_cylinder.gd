@tool
extends Node3D

var radius: float = 1.0
var aspect_ratio = 0.5
var angle: float = 90.0  # Central angle in degrees
var segments: int = 10  # Number of segments along the curved surface

func _ready() -> void:
	var angle_radians = deg_to_rad(angle)
	var arc_length = radius * angle_radians
	var height = aspect_ratio * arc_length
	$MeshInstance3D.mesh = generate_cylinder_portion_mesh(radius, height, angle, segments)

func generate_cylinder_portion_mesh(radius: float, height: float, angle: float, segments: int) -> ArrayMesh:
	var mesh = ArrayMesh.new()
	var arrays = []
	arrays.resize(ArrayMesh.ARRAY_MAX)

	var vertices = PackedVector3Array()
	var normals = PackedVector3Array()
	var uvs = PackedVector2Array()
	var indices = PackedInt32Array()

	var delta_angle = deg_to_rad(angle) / segments
	var half_height = height / 2

	var start_angle = deg_to_rad(-90 - (angle / 2.0))

	# Generate vertices, normals, and uvs
	for i in range(segments + 1):
		var current_angle = start_angle + (i * delta_angle)
		var x = radius * cos(current_angle)
		var z = radius * sin(current_angle)
		var normal = Vector3(cos(current_angle), 0, sin(current_angle))

		vertices.append(Vector3(x, -half_height, z))
		normals.append(normal)
		uvs.append(Vector2(float(i) / segments, 1))

		vertices.append(Vector3(x, half_height, z))
		normals.append(normal)
		uvs.append(Vector2(float(i) / segments, 0))

	# Generate indices for triangles
	for i in range(segments):
		var index = i * 2
		indices.append(index)
		indices.append(index + 1)
		indices.append(index + 3)
		indices.append(index)
		indices.append(index + 3)
		indices.append(index + 2)

	arrays[ArrayMesh.ARRAY_VERTEX] = vertices
	arrays[ArrayMesh.ARRAY_NORMAL] = normals
	arrays[ArrayMesh.ARRAY_TEX_UV] = uvs
	arrays[ArrayMesh.ARRAY_INDEX] = indices

	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
	return mesh
