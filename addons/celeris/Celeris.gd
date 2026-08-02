class_name Celeris
extends Node

signal server_started(host: String, port: int)
signal server_stopped()

var host: String = "127.0.0.1"
var port: int = 8080
var _is_running: bool = false

var _server: TCPServer
var _clients: Array = []
var _client_buffers: Dictionary = {}
var _routes: Dictionary = {}


func _init(p_host: String = "127.0.0.1", p_port: int = 8080) -> void:
	host = p_host
	port = p_port
	var main_loop = Engine.get_main_loop()
	if main_loop and main_loop.root:
		main_loop.root.add_child.call_deferred(self)


func start_server(p_port: int = -1, p_host: String = "") -> void:
	if _is_running:
		push_warning("[Celeris] Server is already running.")
		return

	if p_port > 0:
		port = p_port
	if not p_host.is_empty():
		host = p_host

	_server = TCPServer.new()
	var err = _server.listen(port, host)
	if err != OK:
		push_error("[Celeris] Failed to start server: %s" % err)
		return

	_is_running = true
	print_rich("[color=cyan][Celeris][/color] Server starting on http://%s:%d..." % [host, port])
	server_started.emit(host, port)


func stop_server() -> void:
	if not _is_running:
		return

	_is_running = false
	if _server:
		_server.stop()
		_server = null
	for client in _clients:
		client.disconnect_from_host()
	_clients.clear()
	_client_buffers.clear()
	print_rich("[color=yellow][Celeris][/color] Server stopped.")
	server_stopped.emit()


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


func _process(_delta: float) -> void:
	if not _is_running or _server == null:
		return

	while _server.is_connection_available():
		var client = _server.take_connection()
		_clients.append(client)
		_client_buffers[client] = ""

	var to_remove = []
	for client in _clients:
		if client.get_status() != StreamPeerTCP.STATUS_CONNECTED:
			to_remove.append(client)
			continue

		var available = client.get_available_bytes()
		if available > 0:
			var data = client.get_data(available)[1].get_string_from_utf8()
			_client_buffers[client] += data

			var buffer = _client_buffers[client]
			var header_end = buffer.find("\r\n\r\n")
			if header_end != -1:
				var header_part = buffer.substr(0, header_end)
				var body_part = buffer.substr(header_end + 4)

				var request = _parse_request(header_part, body_part)
				if request != null:
					_log_request(request)
					var response = _handle_request(request)
					_log_response(request, response)
					_send_response(client, response)
					to_remove.append(client)
				else:
					to_remove.append(client)

	for client in to_remove:
		if client in _clients:
			_clients.erase(client)
			_client_buffers.erase(client)
			client.disconnect_from_host()


func _parse_request(header_str: String, body_str: String) -> CelerisRequest:
	var lines = header_str.split("\r\n")
	if lines.size() < 1:
		return null

	var request_line = lines[0].split(" ")
	if request_line.size() < 3:
		return null

	var req = CelerisRequest.new()
	req.method = request_line[0]
	req.path = request_line[1]

	var query_pos = req.path.find("?")
	if query_pos != -1:
		var query_str = req.path.substr(query_pos + 1)
		req.path = req.path.substr(0, query_pos)
		for pair in query_str.split("&"):
			var kv = pair.split("=")
			if kv.size() == 2:
				req.query_params[kv[0]] = kv[1]

	for i in range(1, lines.size()):
		var line = lines[i]
		var sep = line.find(":")
		if sep != -1:
			var key = line.substr(0, sep).strip_edges()
			var value = line.substr(sep + 1).strip_edges()
			req.headers[key] = value

	req.body = body_str
	return req

func _handle_request(req: CelerisRequest) -> CelerisResponse:
	var method_routes = _routes.get(req.method, [])
	var path_parts = req.path.split("/")

	for route in method_routes:
		var pattern = route["pattern"]
		var param_names = route["param_names"]
		if path_parts.size() != pattern.size():
			continue

		var params = {}
		var matches = true
		for i in range(pattern.size()):
			var p = pattern[i]
			var part = path_parts[i]
			if p.begins_with(":"):
				var name = p.substr(1)
				params[name] = part
			elif p != part:
				matches = false
				break

		if matches:
			req.params = params
			var handler = route["handler"]
			if not handler is Callable:
				var res = CelerisResponse.new()
				res.status_code = 500
				res.body = "Internal Server Error: handler not callable"
				return res
			var response = handler.call(req)
			if response == null or not response is CelerisResponse:
				var res = CelerisResponse.new()
				res.status_code = 500
				res.body = "Internal Server Error: handler did not return CelerisResponse"
				return res
			return response

	var res = CelerisResponse.new()
	res.status_code = 404
	res.body = "404 Not Found"
	return res


func _send_response(client: StreamPeerTCP, res: CelerisResponse) -> void:
	var status_text = _get_status_text(res.status_code)
	var response_str = "HTTP/1.1 %d %s\r\n" % [res.status_code, status_text]
	for key in res.headers:
		response_str += "%s: %s\r\n" % [key, res.headers[key]]
	response_str += "\r\n"
	response_str += res.body
	client.put_data(response_str.to_utf8_buffer())
	client.disconnect_from_host()


func _log_request(req: CelerisRequest) -> void:
	var body_preview = req.body if req.body.length() <= 1000 else req.body.substr(0, 1000) + "..."
	print_rich("[color=cyan][Celeris][/color] [bold]%s[/bold] %s" % [req.method, req.path])
	if not req.query_params.is_empty():
		print("[Celeris] Query params: ", req.query_params)
	if not req.headers.is_empty():
		print("[Celeris] Headers: ", req.headers)
	if not req.body.is_empty():
		print("[Celeris] Body: ", body_preview)


func _log_response(req: CelerisRequest, res: CelerisResponse) -> void:
	var status_text = _get_status_text(res.status_code)
	var body_preview = res.body if res.body.length() <= 1000 else res.body.substr(0, 1000) + "..."
	print_rich("[color=green][Celeris][/color] [bold]%s[/bold] %s -> %d %s" % [req.method, req.path, res.status_code, status_text])
	if not res.body.is_empty():
		print("[Celeris] Response body: ", body_preview)


func _get_status_text(code: int) -> String:
	match code:
		200: return "OK"
		201: return "Created"
		400: return "Bad Request"
		401: return "Unauthorized"
		404: return "Not Found"
		500: return "Internal Server Error"
		_: return "Unknown"


func _notification(what: int) -> void:
	if what == NOTIFICATION_PREDELETE or what == NOTIFICATION_UNPARENTED:
		if _is_running:
			stop_server()

func mount(router: CelerisRouter) -> void:
	var router_routes = router.get_routes()
	for method in router_routes:
		if not _routes.has(method):
			_routes[method] = []
		_routes[method] += router_routes[method]
