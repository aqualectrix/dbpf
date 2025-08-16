# Full flattened-tree representation of a BSOK <role>
class BSOKSpec:
	def __init__(self, genre, style, group, shape, role):
		self.genre = genre.strip()
		self.style = style.strip()
		self.group = group
		self.shape = shape.strip()
		self.role = role

	def __str__(self):
		return ("genre: " + self.genre +
		       " style: " + self.style +
			   " group: " + str(self.group) + 
			   " shape: " + self.shape +
			   " role: " + str(self.role))

# Holds the attributes of a BSOK <group>
class BSOKGroup:
	def __init__(self, name, gender):
		self.name = name.strip()
		self.gender = gender.strip()

	def __str__(self):
		return ("<" + 
		       " name: " + self.name +
			   " gender: " + self.gender +
			   " >" )

# Holds the attributes of a BSOK <role>
class BSOKRole:
	def __init__(self, name, code):
		self.name = name.strip()
		self.code = int(code, 16)

	def __str__(self):
		return ("<" +
			   " name: " + self.name +
		       " code (hex): " + hex(self.code) + 
			   " >" )