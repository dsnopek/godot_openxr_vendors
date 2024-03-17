@tool
extends Node3D

var radius = 5
var central_horizontal_angle = 90
var upper_vertical_angle = 45
var lower_vertical_angle = -45
var segments = 32

func _ready() -> void:
	$MeshInstance3D.mesh = generate_equirect_portion_mesh()

func generate_equirect_portion_mesh():
	var mesh = ArrayMesh.new()

	var vertices := PackedVector3Array()
	var normals := PackedVector3Array()
	var uvs := PackedVector2Array()
	var indices := PackedInt32Array()

	var step_horizontal = deg_to_rad(central_horizontal_angle) / segments
	var step_vertical = deg_to_rad(upper_vertical_angle - lower_vertical_angle) / segments

	var start_horizontal_angle = deg_to_rad(90 + (central_horizontal_angle / 2.0))


	for i in range(segments + 1):
		for j in range(segments + 1):
			var horizontal_angle = start_horizontal_angle + (i * step_horizontal)
			var vertical_angle = lower_vertical_angle + j * step_vertical

			var x = radius * cos(vertical_angle) * sin(horizontal_angle)
			var y = radius * sin(vertical_angle)
			var z = radius * cos(vertical_angle) * cos(horizontal_angle)

			vertices.append(Vector3(x, y, z))
			normals.append(Vector3(x, y, z).normalized())
			uvs.append(Vector2(float(i) / segments, float(j) / segments))

	for i in range(segments):
		for j in range(segments):
			var index = i * (segments + 1) + j
			indices.append(index)
			indices.append(index + segments + 1)
			indices.append(index + segments + 2)

			indices.append(index)
			indices.append(index + segments + 2)
			indices.append(index + 1)

	var arrays := []
	arrays.resize(ArrayMesh.ARRAY_MAX)

	arrays[ArrayMesh.ARRAY_VERTEX] = vertices
	arrays[ArrayMesh.ARRAY_NORMAL] = normals
	arrays[ArrayMesh.ARRAY_TEX_UV] = uvs
	arrays[ArrayMesh.ARRAY_INDEX] = indices

	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
	return mesh
