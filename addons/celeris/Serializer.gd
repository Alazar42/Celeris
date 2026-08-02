class_name Serializer
extends RefCounted

static func decode_value(v):
	if v is PackedByteArray:
		return v.get_string_from_ascii()
	if v is Array:
		# If array of ints (bytes), convert to string
		var ok = true
		for x in v:
			if not (typeof(x) == TYPE_INT):
				ok = false
				break
		if ok:
			var bytes = PackedByteArray(v)
			return bytes.get_string_from_ascii()
		return v
	if v is String:
		# Handle stringified byte-array like "[50, 48, ...]"
		if v.begins_with("[") and v.ends_with("]"):
			var inner = v.substr(1, v.length() - 2)
			var parts = inner.split(",")
			var vals: Array = []
			for p in parts:
				var s = p.strip_edges()
				if s == "":
					continue
				if s.is_valid_integer():
					vals.append(int(s))
				else:
					return v
			if vals.size() > 0:
				return PackedByteArray(vals).get_string_from_ascii()
		return v
	return v

static func decode_dict(d: Dictionary) -> Dictionary:
	var out = {}
	for k in d:
		out[k] = decode_value(d[k])
	return out

static func decode_array(arr: Array) -> Array:
	var out: Array = []
	for v in arr:
		if v is Dictionary:
			out.append(decode_dict(v))
		elif v is Array:
			out.append(decode_array(v))
		else:
			out.append(decode_value(v))
	return out
