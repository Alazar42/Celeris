class_name TodoModel
extends CelerisModel

static func _get_model_name() -> String:
	return "todos"

static func _get_schema() -> Dictionary:
	return {
		"id": {"type": "SERIAL", "primary_key": true},
		"title": {"type": "TEXT", "not_null": true},
		"completed": {"type": "BOOLEAN", "default": false},
		"created_at": {"type": "TIMESTAMP", "default": "CURRENT_TIMESTAMP"}
	}
