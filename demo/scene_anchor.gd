extends Node3D

const GRID_MATERIAL: StandardMaterial3D = preload("res://assets/cross-grid-material.tres")

@onready var label: Label3D = $Label3D
@onready var static_body: StaticBody3D = $StaticBody3D

var mesh_instance: MeshInstance3D
var spatial_entity: OpenXRFbSpatialEntity

func setup_scene(p_spatial_entity: OpenXRFbSpatialEntity) -> void:
	spatial_entity = p_spatial_entity

	var semantic_labels: PackedStringArray = spatial_entity.get_semantic_labels()

	label.text = ", ".join(Array(semantic_labels).map(func (x): return x.capitalize()))

	var collision_shape = spatial_entity.create_collision_shape()
	if collision_shape:
		static_body.add_child(collision_shape)

	mesh_instance = spatial_entity.create_mesh_instance()
	if not mesh_instance:
		mesh_instance = MeshInstance3D.new()
		var box_mesh := BoxMesh.new()
		box_mesh.size = Vector3(0.1, 0.1, 0.1)
		mesh_instance.mesh = box_mesh

	# Adjust the material for the entity type.
	var material: StandardMaterial3D = GRID_MATERIAL.duplicate()
	if semantic_labels.size() > 0:
		material.albedo_color = _get_color_for_label(semantic_labels[0])
	if mesh_instance.mesh is BoxMesh:
		material.uv1_scale = Vector3(3, 2, 1)
	mesh_instance.set_surface_override_material(0, material)

	add_child(mesh_instance)

func _get_color_for_label(semantic_label) -> Color:
	match semantic_label:
		"ceiling","floor":
			return Color(0.0, 0.0, 0.0, 1.0)
		"wall_face","invisible_wall_face":
			return Color(0.0, 0.0, 1.0, 1.0)
		"window_frame","door_frame":
			return Color(1.0, 0.0, 0.0, 1.0)
		"couch","table","bed","lamp","plant","screen","storage":
			return Color(0.0, 1.0, 0.0, 1.0)

	return Color(1.0, 1.0, 1.0, 1.0)
