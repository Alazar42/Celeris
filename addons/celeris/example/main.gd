extends Node

var app: Celeris
var todo_router: TodoRouter

func _ready() -> void:
	var driver = PostgreSQLDriver.new()
	DatabaseService.set_driver(driver)

	var conn_str = "your postgres url"
	if not DatabaseService.connect_db(conn_str, true):
		print("Failed to connect to PostgreSQL")
		return

	var schema = TodoModel._get_schema()
	DatabaseService.create_table(TodoModel._get_model_name(), schema)

	app = Celeris.new("127.0.0.1", 8080)
	todo_router = TodoRouter.new()
	app.mount(todo_router)
	app.start_server()
	print("Todo API with PostgreSQL running at http://127.0.0.1:8080")
