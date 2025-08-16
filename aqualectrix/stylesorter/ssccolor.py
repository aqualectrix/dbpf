# Full flattened-tree representation of an SSC Color <color>
class SSCColorSpec:
	def __init__(self, group, style, order, color):
		self.group = group.strip()
		self.style = style.strip()
		self.order = order.strip()
		self.color = color

	def __str__(self):
		return ("group: " + self.group +
			   " style: " + self.style +
			   " order: " + self.order +
			   " color: " + str(self.color))

# Holds the attributes of an SSC Color <color>
class SSCColor:
	def __init__(self, name, code):
		self.name = name.strip()
		self.code = int(code, 16)

	def __str__(self):
		return ("<" +
		       " name: " + self.name +
			   " code (hex): " + hex(self.code) +
			   " >")