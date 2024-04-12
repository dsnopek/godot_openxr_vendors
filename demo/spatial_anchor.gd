extends Area3D

@onready var mesh_instance: MeshInstance3D = $MeshInstance3D

func setup_scene(spatial_entity: OpenXRFbSpatialEntity) -> void:
	var data: Dictionary = spatial_entity.custom_data
	var color: Color = data.get('color', Color(1.0, 1.0, 1.0, 1.0))

	var material: StandardMaterial3D = mesh_instance.get_surface_override_material(0)
	material.albedo_color = color
	mesh_instance.set_surface_override_material(0, material)
