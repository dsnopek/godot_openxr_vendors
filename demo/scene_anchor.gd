extends Node3D

@onready var mesh_instance: MeshInstance3D = $MeshInstance3D

func setup_scene(entity: OpenXRFbSpatialEntity) -> void:
	if entity.is_component_enabled(OpenXRFbSpatialEntity.COMPONENT_TYPE_BOUNDED_3D):
		var bb: AABB = entity.get_bounding_box_3d()
		var mesh: BoxMesh = BoxMesh.new()
		mesh.size = bb.size
		mesh_instance.mesh = mesh
		mesh_instance.position = bb.get_center()
