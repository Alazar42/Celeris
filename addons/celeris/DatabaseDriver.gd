class_name DatabaseDriver
extends RefCounted

func connect_db(connection_string: String, use_ssl: bool = false) -> bool:
	push_error("Not implemented")
	return false

func disconnect_db() -> void:
	push_error("Not implemented")

func execute(query: String) -> Array:
	push_error("Not implemented")
	return []

func insert(table: String, data: Dictionary) -> int:
	push_error("Not implemented")
	return -1

func select(table: String, conditions: Dictionary = {}, limit: int = -1) -> Array:
	push_error("Not implemented")
	return []

func update(table: String, id: int, data: Dictionary) -> bool:
	push_error("Not implemented")
	return false

func delete(table: String, id: int) -> bool:
	push_error("Not implemented")
	return false

func create_table(table: String, schema: Dictionary) -> bool:
	push_error("Not implemented")
	return false