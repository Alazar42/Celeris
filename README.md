# Celeris – REST Framework for Godot 4

![Logo](https://i.ibb.co/cSKHnt8z/celeris-with-name.jpg)

Celeris is a lightweight, declarative REST framework for Godot 4 that makes building HTTP servers and APIs dead simple. Inspired by FastAPI's philosophy, Celeris lets you define models and routes with minimal boilerplate.

## Features

- **Declarative Models**: Define schema once, get automatic CRUD operations.
- **Built-in Database Support**: Works seamlessly with PostgreSQL via a generic database abstraction layer.
- **Minimal Boilerplate**: Controllers call the database service directly; models declare structure, not behavior.
- **Automatic Serialization**: Byte arrays and complex types are automatically decoded and JSON-serialized correctly.
- **HTTP Server**: Fast, built-in TCP-based HTTP server with routing and logging.

## Installation

1. Clone or download the Celeris addon into your Godot project's `addons/` folder.
2. Enable the plugin via **Project → Project Settings → Plugins**.
3. Restart Godot.

## Quick Start

### 1. Define a Model

Create `TodoModel.gd` in your project:

```gdscript
extends CelerisModel

static func _get_model_name() -> String:
    return "todos"

static func _get_schema() -> Dictionary:
    return {
        "id": {"type": "integer", "primary_key": true},
        "title": {"type": "text", "default": ""},
        "completed": {"type": "boolean", "default": false},
        "created_at": {"type": "timestamp", "default": "now()"}
    }
```

That's it! No CRUD methods to override.

### 2. Create a Controller

Create `TodoController.gd`:

```gdscript
extends CelerisController

func list(request: CelerisRequest) -> CelerisResponse:
    var table = TodoModel._get_model_name()
    var rows = DatabaseService.select(table)
    
    var todos = []
    for row in rows:
        todos.append(TodoModel.new(Serializer.decode_dict(row)))
    
    return json(todos)

func create(request: CelerisRequest) -> CelerisResponse:
    var data = request.json_body
    var result = DatabaseService.insert(TodoModel._get_model_name(), data)
    
    if result and not result.is_empty():
        var rows = DatabaseService.select(TodoModel._get_model_name(), {"id": result["id"]})
        if not rows.is_empty():
            return json(TodoModel.new(Serializer.decode_dict(rows[0])), 201)
    
    return json({"error": "Failed to create todo"}, 400)

func get_one(request: CelerisRequest) -> CelerisResponse:
    var id = request.url_params.get("id")
    var rows = DatabaseService.select(TodoModel._get_model_name(), {"id": id})
    
    if rows.is_empty():
        return json({"error": "Not found"}, 404)
    
    return json(TodoModel.new(Serializer.decode_dict(rows[0])))

func update(request: CelerisRequest) -> CelerisResponse:
    var id = request.url_params.get("id")
    var data = request.json_body
    
    DatabaseService.update(TodoModel._get_model_name(), {"id": id}, data)
    var rows = DatabaseService.select(TodoModel._get_model_name(), {"id": id})
    
    if not rows.is_empty():
        return json(TodoModel.new(Serializer.decode_dict(rows[0])))
    
    return json({"error": "Not found"}, 404)

func delete(request: CelerisRequest) -> CelerisResponse:
    var id = request.url_params.get("id")
    DatabaseService.delete(TodoModel._get_model_name(), {"id": id})
    return json({"message": "Deleted"}, 204)
```

### 3. Register Routes

In your main server script or `_ready()` function:

```gdscript
var server = Celeris.new()

# Register CRUD routes
server.add_route("POST", "/api/todos", TodoController.new(), "create")
server.add_route("GET", "/api/todos", TodoController.new(), "list")
server.add_route("GET", "/api/todos/{id}", TodoController.new(), "get_one")
server.add_route("PUT", "/api/todos/{id}", TodoController.new(), "update")
server.add_route("DELETE", "/api/todos/{id}", TodoController.new(), "delete")

server.run(3001)
```

### 4. Connect to PostgreSQL

Initialize the database driver before running the server:

```gdscript
var driver = PostgreSQLDriver.new()
driver.connect_to_database("localhost", 5432, "postgres", "password", "mydb")
DatabaseService.set_driver(driver)

# Create tables if needed
var todo_model = TodoModel.new()
driver.create_table(TodoModel._get_model_name(), TodoModel._get_schema())
```

## Architecture

### Core Components

- **`CelerisModel`**: Base class for all models. Provides schema definition and automatic database-backed CRUD.
- **`CelerisController`**: Base class for all controllers. Handles HTTP request/response serialization.
- **`CelerisRouter`**: Router for registering and dispatching HTTP requests to controllers.
- **`DatabaseService`**: Static database wrapper that routes queries to the active driver.
- **`Serializer`**: Centralized utility for decoding byte arrays and complex types before JSON serialization.

### Request/Response Flow

```
HTTP Request
    ↓
Celeris.route_dispatch()
    ↓
CelerisRouter.match_route()
    ↓
Controller.method()
    ↓
DatabaseService.select/insert/update/delete()
    ↓
PostgreSQLDriver (or other driver)
    ↓
Model instance (via Serializer.decode_dict)
    ↓
CelerisController.json()
    ↓
HTTP Response (JSON)
```

## API Reference

### CelerisModel

#### Static Methods

- `_get_model_name() -> String`: Return the database table name.
- `_get_schema() -> Dictionary`: Return the schema definition.

#### Instance Methods

- `create(data: Dictionary) -> CelerisModel`: Insert a new record and return the model.
- `find(id: int) -> CelerisModel`: Find a record by ID.
- `all() -> Array`: Fetch all records.
- `update(data: Dictionary) -> CelerisModel`: Update the record.
- `delete()`: Delete the record.

### CelerisController

#### Methods

- `json(data, status_code: int = 200) -> CelerisResponse`: Serialize data to JSON and return HTTP response.

### DatabaseService

#### Static Methods

- `set_driver(driver)`: Set the active database driver.
- `select(table: String, where: Dictionary = {}) -> Array`: Fetch rows from table.
- `insert(table: String, data: Dictionary) -> Dictionary`: Insert a row, return inserted row.
- `update(table: String, where: Dictionary, data: Dictionary)`: Update rows matching condition.
- `delete(table: String, where: Dictionary)`: Delete rows matching condition.

### Serializer

#### Static Methods

- `decode_value(value)`: Decode a single value (PackedByteArray, Array of ints, or stringified array).
- `decode_dict(data: Dictionary) -> Dictionary`: Recursively decode all values in a dictionary.
- `decode_array(data: Array) -> Array`: Recursively decode all elements in an array.

## Example Project

See the `example/` folder for a complete todo CRUD example with models, controllers, and routes.

## Troubleshooting

### `GET /api/todos` returns empty array

Ensure that:
1. The database driver is connected and initialized.
2. The table exists (call `driver.create_table()`).
3. The model's `_get_model_name()` matches the actual table name in the database.

### Byte arrays returned in responses

This should not happen; `CelerisController.json()` automatically decodes byte arrays via `Serializer.decode_dict()`. If you see byte arrays in the response, ensure you're calling `json()` and not manually stringifying with `JSON.stringify()`.

### Static method dispatch issues

Controllers should call `DatabaseService` directly with `Model._get_model_name()`, not use model instance methods in static contexts. This avoids GDScript static dispatch limitations.

## Philosophy

Celeris follows these principles:

1. **Declare, don't code**: Models define schema; behavior is inherited.
2. **Minimal boilerplate**: Controllers are thin wrappers around database calls.
3. **Type safety where it matters**: Schema validation at the database level.
4. **Fast defaults**: Built-in HTTP server and serialization; no external dependencies (except PostgreSQL client addon).

## License

MIT License – See [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please submit issues or pull requests to improve Celeris.

---

**Made with ❤️ for Godot developers.**
