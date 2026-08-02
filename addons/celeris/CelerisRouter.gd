class_name CelerisRouter
extends RefCounted

var _routes: Dictionary = {}

func get_route(path: String, handler: Callable) -> void:
	_register_route("GET", path, handler)

func post_route(path: String, handler: Callable) -> void:
	_register_route("POST", path, handler)

func put_route(path: String, handler: Callable) -> void:
	_register_route("PUT", path, handler)

func delete_route(path: String, handler: Callable) -> void:
	_register_route("DELETE", path, handler)

func _register_route(method: String, path: String, handler: Callable) -> void:
	if not _routes.has(method):
		_routes[method] = []
	var parts = path.split("/")
	var pattern = []
	var param_names = []
	for part in parts:
		if part.begins_with(":"):
			pattern.append(":" + part.substr(1))
			param_names.append(part.substr(1))
		else:
			pattern.append(part)
	_routes[method].append({
		"pattern": pattern,
		"param_names": param_names,
		"handler": handler,
		"original": path
	})

func get_routes() -> Dictionary:
	return _routes