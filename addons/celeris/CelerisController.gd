class_name CelerisController
extends RefCounted

func json(data, status_code: int = 200) -> CelerisResponse:
	var res = CelerisResponse.new()
	res.status_code = status_code
	res.headers["Content-Type"] = "application/json"

	# Decode according to runtime type to avoid static type-check issues
	if data is Dictionary:
		res.body = JSON.stringify(Serializer.decode_dict(data))
	elif data is Array:
		res.body = JSON.stringify(Serializer.decode_array(data))
	else:
		# Scalars, PackedByteArray, or string representations
		res.body = JSON.stringify(Serializer.decode_value(data))

	return res

func text(content: String, status_code: int = 200) -> CelerisResponse:
	var res = CelerisResponse.new()
	res.status_code = status_code
	res.headers["Content-Type"] = "text/plain"
	res.body = content
	return res