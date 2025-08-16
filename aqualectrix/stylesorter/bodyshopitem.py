import os.path

class BodyShopItem:
	
	def __init__(self, group, instance, instance2, binx_filename, sortindex, gzps_filename, product, name) :
		self.group = group
		self.instance = instance
		self.instance2 = instance2

		self.binx_filename = binx_filename
		self.sortindex = sortindex

		self.gzps_filename = gzps_filename
		self.product = product
		self.name = name

	def palette(self):
		if not self.sortindex:
			return None
		# Palette is held in the last two hexits of the sortindex: 0x000000pp
		return int(self.sortindex, 10) % (16**2)

	def color(self):
		if not self.sortindex:
			return None
		# Color is held in the 6th and 7th hexits of the sortindex: 0x0000cc00
		color_and_palette = int(self.sortindex, 10) % (16**4)
		return color_and_palette // (16**2)

	def stringify(self, pretty = False, names_config = None):
		item_str = "Group: " + str(self.group)
		
		item_str += " Product: "
		if not pretty or not names_config or int(self.product, 10) not in names_config["BSOK Products"]:
			item_str += self.product
		else:
			item_str += names_config["BSOK Products"][int(self.product, 10)]


		item_str += " SortIndex: "
		if not pretty:
			item_str += self.sortindex
		else:
			if not names_config:
				item_str += str(self.color()) + ":" + str(self.palette())
			else:
				if not names_config["Colors"] or self.color() not in names_config["Colors"]:
					item_str += str(self.color())
				else:
					item_str += names_config["Colors"][self.color()]
				item_str += ":"
				if not names_config["Color Palettes"] or self.palette() not in names_config["Color Palettes"]:
					item_str += str(self.palette())
				else:
					item_str += names_config["Color Palettes"][self.palette()]

		

		item_str += " Name: " + self.name

		item_str += " GZPS Filename: " + os.path.split(self.gzps_filename)[1]

		return item_str

	def __str__(self):
		return self.stringify()
		#return "Group: " + str(self.group) + " Product: " + str(self.product) + " SortIndex: " + str(self.sortindex) + " Name: " + str(self.name) + " GZPS Filename: " + os.path.split(self.gzps_filename)[1]

	# Here, "less than" means "sorts earlier than"
	def __lt__(self, other):
		# We sort unknowns to the beginning; that is, unknown product is < 0
		if not hasattr(self, "product") or not self.product or self.product == "UNKNOWN":
			return True

		# Product 0 comes first, then products from higher to lower values
		if self.product == "0" and other.product != "0":
			return True
		if self.product != "0" and other.product == "0":
			return False
		if self.product != other.product:
			# Yes, this is the correct direction -- higher products sort earlier
			return int(self.product, 10) > int(other.product, 10)
		
		# Products are equal; check the sortindex

		# Sort unknowns to the beginning
		if not hasattr(self, "sortindex") or not self.sortindex or self.sortindex == "UNKNOWN":
			return True

		# Within product 0, higher sortindexes come before lower ones
		# Within other products, lower sortindexes come before higher ones
		if self.product == "0":
			return int(self.sortindex, 10) > int(other.sortindex, 10)
		else:
			return int(self.sortindex, 10) < int(other.sortindex, 10)
		