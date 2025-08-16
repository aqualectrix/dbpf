# Full flattened-tree representation of an SSC Palette <palette>
class SSCPaletteSpec:
	def __init__(self, group, style, order, palette):
		self.group = group.strip()
		self.style = style.strip()
		self.order = order.strip()
		self.palette = palette

	def __str__(self):
		return ("group: " + self.group +
			   " style: " + self.style +
			   " order: " + self.order +
			   " palette: " + str(self.palette))

# Holds the attributes of an SSC Palette <palette>
class SSCPalette:
	def __init__(self, name, code):
		self.name = name.strip()
		self.code = int(code, 16)

	def __str__(self):
		return ("<" +
		       " name: " + self.name +
			   " code (hex): " + hex(self.code) +
			   " >")