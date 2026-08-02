class_name TodoController
extends CelerisController

func list(req: CelerisRequest) -> CelerisResponse:
	var table = TodoModel._get_model_name()
	var rows = DatabaseService.select(table)
	var todos: Array = []
	for r in rows:
		todos.append(TodoModel.new(Serializer.decode_dict(r)))
	return json({"todos": todos.map(func(t): return t.to_dictionary())})

func create(req: CelerisRequest) -> CelerisResponse:
	var payload = req.json()
	var data = {
		"title": payload.get("title", "Untitled"),
		"completed": payload.get("completed", false)
	}
	var table = TodoModel._get_model_name()
	var inserted_id = DatabaseService.insert(table, data)
	if inserted_id < 0:
		return json({"error": "Failed to create todo"}, 500)
	var rows = DatabaseService.select(table, {"id": inserted_id}, 1)
	var created = TodoModel.new(data.duplicate(true)) if rows.is_empty() else TodoModel.new(Serializer.decode_dict(rows[0]))
	return json(created.to_dictionary(), 201)

func update(req: CelerisRequest) -> CelerisResponse:
	var id = int(req.params.get("id", "0"))
	var payload = req.json()
	var table = TodoModel._get_model_name()
	if DatabaseService.update(table, id, payload):
		var rows = DatabaseService.select(table, {"id": id}, 1)
		if not rows.is_empty():
			return json(TodoModel.new(Serializer.decode_dict(rows[0])).to_dictionary())
	return json({"error": "Todo not found"}, 404)

func delete(req: CelerisRequest) -> CelerisResponse:
	var id = int(req.params.get("id", "0"))
	var table = TodoModel._get_model_name()
	if DatabaseService.delete(table, id):
		return json({"message": "Todo deleted"})
	return json({"error": "Todo not found"}, 404)
