import bodyShopProcessWrapper
import argparse
import glob
from enum import Enum
from bodyshopitem import *
import configparser
import xml.etree.ElementTree as ET
from bsokproduct import *
from ssccolor import *
from sscpalette import *
import os.path

# Define enum for resource types
ResourceType = Enum('ResourceType', [("DBPF_BINX", 0x0C560F39), ("DBPF_GZPS", 0xEBCF3E27)])

def main(args):
	args = parse_args(args)

	# Load items from selected files
	items = load_items(args.filenames, args.recursive)

	# Load config files
	names_map = {}

	files_config = configparser.ConfigParser()
	files_config.read("config.ini")
	load_bsok(files_config["BSOK Files"], names_map)
	load_ssc(files_config["Color Files"], names_map)

	if args.list:
		for item in sorted(items.values()):
			print(item.stringify(pretty = True, names_config = names_map))
		return

	if args.colors_update:
		new_colors_map = {}
		load_ssc({"xml_filepath": args.colors_update[0], "color_style": args.colors_update[1], "color_order": args.colors_update[2]}, new_colors_map)
		old_new_map = produce_old_to_new_map("Colors", names_map, new_colors_map)
		
		for item in items.values():
			bodyShopProcessWrapper.updateBodyShopItem(item, color_map = old_new_map)
		return

	if args.namesync:
		# Create a name:id color map
		names_map["Color Names"] = {color_name:color_id for color_id, color_name in names_map["Colors"].items()}

		filenames = []
		for path in args.filenames:
			filenames += glob.glob(path, recursive = args.recursive)

		for filename in filenames:
			if not check_valid_namesync_filename(filename, args.namesync[0]):
				continue

			print("Namesyncing", filename)
			bodyShopProcessWrapper.namesync(filename, names_map, args.namesync[1], args.namesync[2])

def parse_args(args):
	parser = argparse.ArgumentParser(prog = "Style Sorter", description = "Sort outfits by style, color, and color palette.")

	parser.add_argument("-l", "--list", action = "store_true", default = False, help = "List selected files in the order they will appear in CAS.")

	parser.add_argument("-c", "--colors_update", nargs=3, metavar =("COLORS_FILE", "COLOR_STYLE", "COLOR_ORDER"), help = "Given an xml file with color definitions, a color style, and color ordering (defined in that file), change the selected files to use the given style and ordering. Colors are matched by name; unmatched colors will not be updated.")
	parser.add_argument('-n', "--namesync", nargs=3, metavar = ("CREATOR_SETNAME", "PRODUCT_ID", "MESHNAME"), help = "For each given file with the prefix CREATOR_SETNAME, edit internal resources and fields to be based on the name of the file (CREATOR_SETNAME_COLOR.package) and the given product id and meshname.")

	parser.add_argument("-r", "--recursive", action = "store_true", default = False, help = "Recurse into any folders found in the list of selected files.")

	parser.add_argument("filenames", help = "File(s) to process", nargs = "*")

	return parser.parse_args(args);

def load_items(globbables, recursive):
	filenames = []
	for path in globbables:
		filenames += glob.glob(path, recursive=recursive)

	items = {}
	for file in filenames:
		data = bodyShopProcessWrapper.extractBodyShopData(file)
		update_items_from_raw(items, file, data)

	return items

def update_items_from_raw(items, filename, rawdata):
	for datum in rawdata:
		# Parse data into more usable form
		group = datum.group
		res_type = datum.type
		instance = datum.instance
		instance2 = datum.instance2
		key = datum.key.decode("utf-8")
		value = datum.value.decode("utf-8")
		
		# Parse resource-specific data
		parsed = {"binx_filename": "",
		          "sortindex": "",
				  "gzps_filename": "",
				  "product": "",
				  "name": ""}
		match res_type:
			case ResourceType.DBPF_BINX.value:
				parsed["binx_filename"] = filename
				match key:
					case "sortindex":
						parsed["sortindex"] = value
					case _:
						print("WARNING: unknown key", key, "in BINX from", filename)
			case ResourceType.DBPF_GZPS.value:
				parsed["gzps_filename"] = filename
				match key:
					case "product":
						parsed["product"] = value
					case "name":
						parsed["name"] = value
					case _:
						print("WARNING: unknown key", key, "in GZPS from", filename)
			case _:
				print("WARNING: unknown resource type", res_type, "from", filename)

		# Create or merge into an existing BodyShopItem
		if (group, instance, instance2) not in items:
			items[(group, instance, instance2)] = BodyShopItem(
										group = group, 
										instance = instance,
										instance2 = instance2,
										binx_filename = parsed["binx_filename"],
										sortindex = parsed["sortindex"],
										gzps_filename = parsed["gzps_filename"],
										product = parsed["product"],
										name = parsed["name"])
		else:
			item = items[(group, instance, instance2)]
			for key, value in parsed.items():
				if value != "":
					if getattr(item, key) and getattr(item, key) != value:
						print("WARNING: Group", group, "Overwriting", key, getattr(item, key), "with", value)
					setattr(item, key, value)

		#print("__")
		#for key, value in items.items():
		#	print("KEY:", key, "VALUE:", value)
		#print("__")

	return items

def load_bsok(bsok_files_config, names_map):
	if "BSOK Products" not in names_map:
		names_map["BSOK Products"] = {}

	bsok_products = []
	for file in bsok_files_config["xml_filepath"].split(";"):
		tree = ET.parse(file)
		bsok = tree.getroot()
		for genre in bsok:
			for style in genre:
				for group in style:
					for shape in group:
						for role in shape:
							bsok_products.append(BSOKSpec(
								genre = genre.attrib["name"],
								style = style.attrib["name"],
								group = BSOKGroup(group.attrib["name"], group.attrib["gender"]),
								shape = shape.attrib["name"],
								role = BSOKRole(role.attrib["name"], role.attrib["code"])))

	for product in bsok_products:
		names_map["BSOK Products"][product.role.code] = product.role.name

						
def load_ssc(ssc_files_config, names_map):
	# Make sure ssc_files_config has all config values
	if "color_style" not in ssc_files_config:
		ssc_files_config["color_style"] = ""
	if "color_order" not in ssc_files_config:
		ssc_files_config["color_order"] = ""
	if "palette_style" not in ssc_files_config:
		ssc_files_config["palette_style"] = ""
	if "palette_order" not in ssc_files_config:
		ssc_files_config["palette_order"] = ""

	# Make sure names_map has the sections we might fill
	if "Colors" not in names_map:
		names_map["Colors"] = {}
	if "Color Palettes" not in names_map:
		names_map["Color Palettes"] = {}

	ssc_colors = []
	ssc_palettes = []
	for file in ssc_files_config["xml_filepath"].split(";"):
		tree = ET.parse(file)
		ssc_color = tree.getroot()
		for group in ssc_color:
			if group.attrib["name"] == "Colors":
				for style in group:
						for order in style:
							for color in order:
								ssc_colors.append(SSCColorSpec(
									group = group.attrib["name"],
									style = style.attrib["name"],
									order = order.attrib["name"],
									color = SSCColor(color.attrib["name"], color.attrib["code"])
								))
			elif group.attrib["name"] == "Palettes":
				for style in group:
					for order in style:
						for palette in order:
							ssc_palettes.append(SSCPaletteSpec(
								group = group.attrib["name"],
								style = style.attrib["name"],
								order = order.attrib["name"],
								palette = SSCPalette(palette.attrib["name"], palette.attrib["code"])
							))

	preferred_color_style = ssc_files_config["color_style"].strip()
	if preferred_color_style not in [spec.style for spec in ssc_colors]:
		print("WARNING: Could not find Color Style", preferred_color_style, "in", ssc_files_config["xml_filepath"], ". Colors will not be identified.")

	preferred_color_order = ssc_files_config["color_order"].strip()
	if preferred_color_order not in [spec.order for spec in ssc_colors]:
		print("WARNING: Could not find Color Order", preferred_color_order, "in", ssc_files_config["xml_filepath"], ". Colors will not be identified.")

	for spec in ssc_colors:
		if spec.style == preferred_color_style and spec.order == preferred_color_order:
			names_map["Colors"][spec.color.code] = spec.color.name

	preferred_palette_style = ssc_files_config["palette_style"].strip()
	if preferred_palette_style not in [spec.style for spec in ssc_palettes]:
		print("WARNING: Could not find Palette Style", preferred_palette_style, "in", ssc_files_config["xml_filepath"], ". Palettes will not be identified.")

	preferred_palette_order = ssc_files_config["palette_order"].strip()
	if preferred_palette_order not in [spec.order for spec in ssc_palettes]:
		print("WARNING: Could not find Palette Order", preferred_palette_order, "in", ssc_files_config["xml_filepath"], ". Palettes will not be identified.")

	for spec in ssc_palettes:
		if spec.style == preferred_palette_style and spec.order == preferred_palette_order:
			names_map["Color Palettes"][spec.palette.code] = spec.palette.name

def produce_old_to_new_map(key, old_names_map, new_names_map):
	# We turn two number:name maps into one number:number map
	# by matching names.
	old_map = old_names_map[key]
	new_map = new_names_map[key]

	return { old_num:new_num for old_num, old_name in old_map.items() for new_num, new_name in new_map.items() if old_name == new_name }

def check_valid_namesync_filename(filename, prefix):
	basename = os.path.basename(filename)
	nameparts = basename.split("_")

	# Names without at least 2 parts cannot match the prefix; they are invalid
	if not len(nameparts) >= 2:
		#print(basename, "is too short.")
		return False

	# Names that don't match the prefix are invalid
	if not prefix == "_".join(nameparts[:2]):
		#print(basename, "doesn't match the prefix", prefix)
		return False

	# Names that match the prefix but don't have exactly 3 parts are invalid;
	# moreover, that's probably not what the user expected, so print a warning.
	if not len(nameparts) == 3:
		print("WARNING:", filename, " matches the prefix but is not formatted as Creator_Setname_Color.package. It will not be processed.")
		return False

	return True

if __name__ == '__main__':
	import sys
	main(sys.argv[1:])