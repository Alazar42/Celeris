class_name CelerisModel
extends RefCounted

static var _store: Dictionary = {}

var _data: Dictionary = {}

func _init(data: Dictionary = {}) -> void:
	var normalized_data: Dictionary = {}
	for key in data:
		normalized_data[String(key)] = data[key]

	var schema = _get_schema()
	for field in schema:
		var def = schema[field]
		var default_value = def.get("default", null)
		_data[field] = normalized_data.get(field, default_value)
	if not _data.has("id") or _data["id"] == null:
		_data["id"] = _generate_id()

static func _generate_id() -> int:
	var model_name = _get_model_name()
	if not _store.has(model_name):
		_store[model_name] = {}
	var ids = _store[model_name].keys()
	var max_id = 0
	for id in ids:
		if id > max_id:
			max_id = id
	return max_id + 1

# Override in subclasses
static func _get_model_name() -> String:
	return "Unknown"

# Override in subclasses
static func _get_schema() -> Dictionary:
	return {}

# Map a database row (Dictionary or Array) into a normalized Dictionary using the model's schema.
static func map_db_row(row) -> Dictionary:
	var schema = _get_schema()
	var out: Dictionary = {}
	var fields: Array = schema.keys()

	if row is Dictionary:
		for field in fields:
			if row.has(field):
				out[field] = Serializer.decode_value(row[field])
			else:
				out[field] = schema[field].get("default", null)
	elif row is Array:
		for i in range(min(row.size(), fields.size())):
			out[fields[i]] = Serializer.decode_value(row[i])
		for field in fields:
			if not out.has(field):
				out[field] = schema[field].get("default", null)
	else:
		# Fallback: return empty defaults
		for field in fields:
			out[field] = schema[field].get("default", null)

	return out

# Create a model instance from a database row
static func from_db_row(row) -> CelerisModel:
	var data = map_db_row(row)
	return new(data)

func get_field(field: String):
	return _data.get(field)

func set_field(field: String, value) -> void:
	var schema = _get_schema()
	if schema.has(field):
		_data[field] = value


func to_dictionary() -> Dictionary:
	return _data.duplicate(true)

func save() -> bool:
	var model_name = _get_model_name()
	if not _store.has(model_name):
		_store[model_name] = {}
	_store[model_name][_data["id"]] = _data.duplicate()
	return true

func destroy() -> bool:
	var model_name = _get_model_name()
	if _store.has(model_name) and _store[model_name].has(_data["id"]):
		_store[model_name].erase(_data["id"])
		return true
	return false

# --- Static CRUD ---

static func create(data: Dictionary) -> CelerisModel:
	# If a DB driver is configured, perform DB-backed create and return mapped model
	if DatabaseService._driver != null:
		return db_create(data)
	var instance = new(data)
	instance.save()
	return instance

static func find(id: int) -> CelerisModel:
	# DB-backed find if driver exists
	if DatabaseService._driver != null:
		return db_find(id)
	var model_name = _get_model_name()
	if _store.has(model_name) and _store[model_name].has(id):
		return new(_store[model_name][id])
	return null

static func all() -> Array:
	# DB-backed all() if driver present
	if DatabaseService._driver != null:
		return db_all()
	var model_name = _get_model_name()
	var instances = []
	if _store.has(model_name):
		for id in _store[model_name]:
			instances.append(new(_store[model_name][id]))
	return instances

static func update(id: int, data: Dictionary) -> bool:
	# DB-backed update if driver exists
	if DatabaseService._driver != null:
		return db_update(id, data)
	var model_name = _get_model_name()
	if _store.has(model_name) and _store[model_name].has(id):
		var record = _store[model_name][id]
		var schema = _get_schema()
		for key in data:
			if schema.has(key):
				record[key] = data[key]
		_store[model_name][id] = record
		return true
	return false

static func delete(id: int) -> bool:
	# DB-backed delete if driver exists
	if DatabaseService._driver != null:
		return db_delete(id)
	var model_name = _get_model_name()
	if _store.has(model_name) and _store[model_name].has(id):
		_store[model_name].erase(id)
		return true
	return false

# --- DB-backed helpers ---
static func db_create(data: Dictionary) -> CelerisModel:
	var table = _get_model_name()
	var inserted_id = DatabaseService.insert(table, data)
	if inserted_id < 0:
		return null
	var rows = DatabaseService.select(table, {"id": inserted_id}, 1)
	if rows.is_empty():
		# return instance constructed from provided data when select failed
		var created = new(data.duplicate(true))
		created.set_field("id", inserted_id)
		return created
	return from_db_row(rows[0])

static func db_find(id: int) -> CelerisModel:
	var table = _get_model_name()
	var rows = DatabaseService.select(table, {"id": id}, 1)
	if rows.is_empty():
		return null
	return from_db_row(rows[0])

static func db_all() -> Array:
	var table = _get_model_name()
	var rows = DatabaseService.select(table)
	var out: Array = []
	for r in rows:
		out.append(from_db_row(r))
	return out

static func db_update(id: int, data: Dictionary) -> bool:
	var table = _get_model_name()
	return DatabaseService.update(table, id, data)

static func db_delete(id: int) -> bool:
	var table = _get_model_name()
	return DatabaseService.delete(table, id)

static func clear_all() -> void:
	var model_name = _get_model_name()
	_store[model_name] = {}