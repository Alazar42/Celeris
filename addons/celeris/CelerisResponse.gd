class_name CelerisResponse
extends RefCounted

var status_code: int = 200
var headers: Dictionary = {
	"Server": "Celeris/1.0",
	"Content-Type": "text/html; charset=utf-8"
}
var body: String = ""

func set_status(code: int) -> CelerisResponse:
	status_code = code
	return self

func set_header(key: String, value: String) -> CelerisResponse:
	headers[key] = value
	return self

func send(text_body: String) -> CelerisResponse:
	body = text_body
	return self