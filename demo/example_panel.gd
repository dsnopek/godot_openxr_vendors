extends Control

@onready var pointer: ColorRect = $Pointer

var start_pos: Vector2

func _ready() -> void:
	start_pos = $Label.position

func _process(delta: float) -> void:
	$Label.position = Vector2(start_pos.x + (100.0 * sin(Time.get_ticks_msec() * 0.001 * 2)), start_pos.y)

func update_pointer(p_position: Vector2) -> void:
	pointer.position = size * p_position
