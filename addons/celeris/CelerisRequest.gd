class_name CelerisRequest
extends RefCounted

var method: String = "GET"
var path: String = "/"
var headers: Dictionary = {}
var query_params: Dictionary = {}
var body: String = ""
var params: Dictionary = {}

func json() -> Dictionary:
	if body.is_empty():
		return {}
	var parsed = JSON.parse_string(body)
	return parsed if typeof(parsed) == TYPE_DICTIONARY else {}