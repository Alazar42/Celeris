class_name DatabaseService
extends RefCounted

static var _driver: DatabaseDriver = null

static func set_driver(driver: DatabaseDriver) -> void:
	_driver = driver

static func connect_db(connection_string: String, use_ssl: bool = false) -> bool:
	if _driver == null:
		push_error("No driver set")
		return false
	return _driver.connect_db(connection_string, use_ssl)

static func disconnect_db() -> void:
	if _driver:
		_driver.disconnect_db()

static func execute(query: String) -> Array:
	if _driver == null:
		return []
	return _driver.execute(query)

static func insert(table: String, data: Dictionary) -> int:
	if _driver == null:
		return -1
	return _driver.insert(table, data)

static func select(table: String, conditions: Dictionary = {}, limit: int = -1) -> Array:
	if _driver == null:
		return []
	return _driver.select(table, conditions, limit)

static func update(table: String, id: int, data: Dictionary) -> bool:
	if _driver == null:
		return false
	return _driver.update(table, id, data)

static func delete(table: String, id: int) -> bool:
	if _driver == null:
		return false
	return _driver.delete(table, id)

static func create_table(table: String, schema: Dictionary) -> bool:
	if _driver == null:
		return false
	return _driver.create_table(table, schema)
