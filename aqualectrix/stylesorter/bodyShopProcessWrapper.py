import ctypes
import os
import pathlib
from enum import Enum

# Load shared library into ctypes
os.add_dll_directory("C:/msys64/ucrt64/bin")
os.add_dll_directory(os.getcwd())
c_lib = ctypes.CDLL("./libBodyShopProcess.so")

# Define return type struct
class RESOURCE_INFO(ctypes.Structure):
	_fields_ = [("group", ctypes.c_uint),
				("type", ctypes.c_uint),
				("instance", ctypes.c_uint),
				("instance2", ctypes.c_uint),
				("key", ctypes.c_char_p),
				("value", ctypes.c_char_p)]

# Provide details about readProcess
# void readProcess (const char* filename, int* infos_length)
c_lib.readProcess.restype = ctypes.POINTER(RESOURCE_INFO)
c_lib.readProcess.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_int)]

def extractBodyShopData(filename):
	infos_count = ctypes.c_int()
	better_infos = c_lib.readProcess(filename.encode('utf-8'), ctypes.byref(infos_count))

	# Slice the returned POINTER(RESOURCE_INFO) to get a Python array
	data = better_infos[:infos_count.value]
	
	# Have the C library free the memory it allocated now that we've copied it via slice
	c_lib.freeResourceInfos(better_infos, infos_count.value)

	return data

# Define update map struct
class UPDATE_MAP_PAIR(ctypes.Structure):
	_fields_ = [("old_val", ctypes.c_uint),
	           ("new_val", ctypes.c_uint)]

# Provide details about updateItemProcess
# bool updateItemProcess (const char* filename, const int group, const int instance, const int instance2, updateMapPair* color_map, int color_map_length)
c_lib.updateItemProcess.restype = ctypes.c_bool
c_lib.updateItemProcess.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.POINTER(UPDATE_MAP_PAIR), ctypes.c_int]

def updateBodyShopItem(item, color_map):
	# We need to update the BINX. If we don't know where it is, there's nothing we can do.
	if not item.binx_filename:
		return

	# Turn the color_map dictionary into a list of UPDATE_MAP_PAIRs that we can pass into C.
	color_map_list = [UPDATE_MAP_PAIR(old_val, new_val) for old_val, new_val in color_map.items()]
	# Turn that list into a proper ctypes array
	color_map_array = (UPDATE_MAP_PAIR * len(color_map_list))(*color_map_list)

	success = c_lib.updateItemProcess(item.binx_filename.encode('utf-8'), item.group, item.instance, item.instance2, ctypes.byref(color_map_array[0]), len(color_map_array))

	print(success)

# Provide details about namesyncProcess
# bool namesyncProcess(const char* filename, const char* creator, const char* setname, const char* colorname, const char* meshname, const char* color_lowercase, const int product_id, const int color_id)
c_lib.namesyncProcess.restype = ctypes.c_bool
c_lib.namesyncProcess.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_uint, ctypes.c_int]

def namesync(filename, namesync_indices, product_id, meshname):
	basename = os.path.basename(filename)
	nameparts = basename.split("_") # earlier validated to have exactly 3 parts
	creator = nameparts[0]
	setname = nameparts[1]

	# Remove the extension from the name of the color
	color = os.path.splitext(nameparts[2])[0]
	
	sortindex = 0
	if color in namesync_indices:
		sortindex = namesync_indices[color]

	success = c_lib.namesyncProcess(filename.encode("utf-8"), creator.encode("utf-8"), setname.encode("utf-8"), color.encode("utf-8"), meshname.encode("utf-8"), color.lower().encode("utf-8"), product_id, sortindex)

# Provide details about freeResourceInfos
# void freeResourceInfos(resourceInfo* infos)
c_lib.freeResourceInfos.restype = None
c_lib.freeResourceInfos.argtypes = [ctypes.POINTER(RESOURCE_INFO), ctypes.c_int]

def cleanupResourceInfos(infos, infos_length):
	c_lib.freeResourceInfos(infos, infos_length)