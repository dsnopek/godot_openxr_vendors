@tool
extends EditorScript

func _run():
	var vertices := PackedVector3Array()
	vertices.resize(3)
	vertices[0] = Vector3(-1.0, -1.0, 1.0);
	vertices[1] = Vector3(3.0, -1.0, 1.0);
	vertices[2] = Vector3(-1.0, 3.0, 1.0);

	var arr := []
	arr.resize(Mesh.ARRAY_MAX)
	arr[Mesh.ARRAY_VERTEX] = vertices

	var mesh := ArrayMesh.new()
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arr)

	ResourceSaver.save(mesh, "res://fullscreen_triangle.tres")
