class_name PostgreSQLDriver
extends DatabaseDriver

var _client: PostgreSQLClient
var _connected: bool = false

var _last_result: Array = []
var _last_error: Dictionary = {}
var _result_ready: bool = false

func _init():
	_client = PostgreSQLClient.new()
	_client.data_received.connect(_on_data_received)
	_client.connection_closed.connect(_on_connection_closed)

func connect_db(connection_string: String, use_ssl: bool = false) -> bool:
	if _connected:
		return true

	print("[PostgreSQLDriver] Connecting to: ", connection_string)
	print("[PostgreSQLDriver] SSL enabled: ", use_ssl)

	var ssl_method = PostgreSQLClient.SecureConnectionMethod.SSL if use_ssl else PostgreSQLClient.SecureConnectionMethod.NONE
	var err = _client.connect_to_host(connection_string, ssl_method)
	if err != OK:
		push_error("PostgreSQL connection failed: ", err)
		return false

	# Wait for connection to complete
	var timeout_ms = 10000  # 10 seconds
	var elapsed = 0
	while _client.status == PostgreSQLClient.Status.STATUS_CONNECTING and elapsed < timeout_ms:
		_client.poll()
		OS.delay_msec(10)
		elapsed += 10
		# Print status every second for debugging
		if elapsed % 1000 == 0:
			print("[PostgreSQLDriver] Still connecting... status: ", _client.status)

	# Check final status
	if _client.status == PostgreSQLClient.Status.STATUS_CONNECTED:
		_connected = true
		print("[PostgreSQLDriver] Connected successfully!")
		return true

	# Connection failed – report detailed error
	if not _client.error_object.is_empty():
		push_error("[PostgreSQLDriver] Connection error: ", _client.error_object)
	else:
		push_error("[PostgreSQLDriver] Connection failed after handshake (status: %d)" % _client.status)
		# Print parameter_status to see if we got any server info
		print("[PostgreSQLDriver] Parameter status: ", _client.parameter_status)
	return false

func disconnect_db() -> void:
	if _client:
		_client.close(true)
		_client = null
	_connected = false

func execute(query: String) -> Array:
	if not _connected or _client.status != PostgreSQLClient.Status.STATUS_CONNECTED:
		push_error("Not connected")
		return []

	print_rich("[color=magenta][PostgreSQL][/color] SQL -> %s" % query)
	_last_result = []
	_last_error = {}
	_result_ready = false

	var err = _client.execute(query)
	if err != OK:
		push_error("Execute failed: ", err)
		return []

	# Wait for the result
	while not _result_ready and _client.status == PostgreSQLClient.Status.STATUS_CONNECTED:
		_client.poll()
		OS.delay_msec(10)

	if not _client.error_object.is_empty():
		push_error("Query error: ", _client.error_object)
		print_rich("[color=red][PostgreSQL][/color] SQL error: %s" % str(_client.error_object))
		return []

	print_rich("[color=magenta][PostgreSQL][/color] SQL rows: %d" % _last_result.size())
	return _last_result

func insert(table: String, data: Dictionary) -> int:
	if data.is_empty():
		push_error("Insert called with empty data")
		return -1

	var columns: Array[String] = []
	var values: Array[String] = []
	for key in data:
		columns.append(key)
		values.append(_escape_value(data[key]))

	var query = "INSERT INTO %s (%s) VALUES (%s) RETURNING id" % [
		table,
		", ".join(columns),
		", ".join(values)
	]
	var result = execute(query)
	if result.is_empty():
		return -1

	var inserted_row = result[0]
	var inserted_id = null
	if inserted_row is Dictionary:
		inserted_id = inserted_row.get("id")
	elif inserted_row is Array and not inserted_row.is_empty():
		inserted_id = inserted_row[0]
	return int(inserted_id) if inserted_id != null else -1

func select(table: String, conditions: Dictionary = {}, limit: int = -1) -> Array:
	var query = "SELECT * FROM %s" % table
	var where_clauses: Array[String] = []
	for key in conditions:
		where_clauses.append("%s = %s" % [key, _escape_value(conditions[key])])
	if where_clauses.size() > 0:
		query += " WHERE " + " AND ".join(where_clauses)
	if limit > 0:
		query += " LIMIT %d" % limit
	return execute(query)

func update(table: String, id: int, data: Dictionary) -> bool:
	if data.is_empty():
		return true

	var assignments: Array[String] = []
	for key in data:
		assignments.append("%s = %s" % [key, _escape_value(data[key])])

	var query = "UPDATE %s SET %s WHERE id = %d" % [table, ", ".join(assignments), id]
	execute(query)
	return _client.error_object.is_empty()

func delete(table: String, id: int) -> bool:
	var query = "DELETE FROM %s WHERE id = %d" % [table, id]
	execute(query)
	return _client.error_object.is_empty()

func create_table(table: String, schema: Dictionary) -> bool:
	var col_defs: Array[String] = []
	for field in schema:
		var def = schema[field]
		var col_def = field + " " + def["type"]
		if def.has("primary_key") and def["primary_key"]:
			col_def += " PRIMARY KEY"
		if def.has("not_null") and def["not_null"]:
			col_def += " NOT NULL"
		if def.has("default"):
			col_def += " DEFAULT " + _sql_default_value(def["default"])
		col_defs.append(col_def)

	var query = "CREATE TABLE IF NOT EXISTS %s (%s)" % [table, ", ".join(col_defs)]
	execute(query)
	return _client.error_object.is_empty()

func _escape_value(value) -> String:
	if value is String:
		return "'" + value.replace("'", "''") + "'"
	elif value is bool:
		return "true" if value else "false"
	elif value == null:
		return "NULL"
	else:
		return str(value)

func _sql_default_value(value) -> String:
	if value is String:
		var cleaned = value.strip_edges()
		if cleaned.is_valid_identifier() or cleaned.ends_with("()"):
			return cleaned
		return "'" + cleaned.replace("'", "''") + "'"
	return _escape_value(value)

func _on_data_received(error_object: Dictionary, transaction_status: int, datas: Array) -> void:
	_last_error = error_object
	if datas.size() > 0:
		var first = datas[0]
		if first is PostgreSQLClient.PostgreSQLQueryResult:
			_last_result = _query_result_to_rows(first)
		else:
			_last_result = []
	else:
		_last_result = []
	_result_ready = true

func _query_result_to_rows(query_result: PostgreSQLClient.PostgreSQLQueryResult) -> Array:
	var rows: Array = []
	if query_result == null:
		return rows

	for row in query_result.data_row:
		var mapped_row: Dictionary = {}
		for index in query_result.row_description.size():
			var field_description = query_result.row_description[index]
			var field_name = String(field_description.get("field_name", ""))
			if field_name != "" and index < row.size():
				mapped_row[field_name] = row[index]
		rows.append(mapped_row)

	return rows

func _on_connection_closed(was_clean_close: bool) -> void:
	_connected = false
	_result_ready = true
