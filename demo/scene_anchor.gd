extends Node3D

@onready var label: Label3D = $Label3D

var mesh_instance: MeshInstance3D

func setup_scene(entity: OpenXRFbSpatialEntity) -> void:
	label.text = ", ".join(entity.get_semantic_labels())

	mesh_instance = entity.create_mesh_instance()
	if not mesh_instance:
		mesh_instance = MeshInstance3D.new()
		var box_mesh := BoxMesh.new()
		box_mesh.size = Vector3(0.1, 0.1, 0.1)
		mesh_instance.mesh = box_mesh
	add_child(mesh_instance)
