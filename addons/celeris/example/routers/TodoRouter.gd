class_name TodoRouter
extends CelerisRouter

# Controller instance – you can also pass it via constructor
var _controller: TodoController

func _init(controller: TodoController = null) -> void:
	if controller:
		_controller = controller
	else:
		_controller = TodoController.new()
	# Register routes
	get_route("/api/todos", _controller.list)
	post_route("/api/todos", _controller.create)
	put_route("/api/todos/:id", _controller.update)
	delete_route("/api/todos/:id", _controller.delete)
