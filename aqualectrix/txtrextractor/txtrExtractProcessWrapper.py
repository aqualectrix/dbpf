import ctypes
import shutil
import os

# Load shared library into ctypes
os.add_dll_directory("C:/msys64/ucrt64/bin")
os.add_dll_directory(os.getcwd())
c_lib = ctypes.CDLL("./libtxtrExtractProcess.so")

# Provide details about txtrExtractProcess
# int txtrExtractProcess(const char** filenames, const int num_filenames, const char* savefile_name)
c_lib.txtrExtractProcess.restype = ctypes.c_int
c_lib.txtrExtractProcess.argtypes = [ctypes.POINTER(ctypes.c_char_p), ctypes.c_int, ctypes.c_char_p]

def extractFromFiles(filenames, savefile_prefix):
	encoded_names = [f.encode('utf-8') for f in filenames]
	file_array = (ctypes.c_char_p * len(filenames))(*encoded_names)
	savefile = savefile_prefix + "TXTRs.package"

	# Create a copy of the empty .package in the same directory as the first file.
	empty_package_path = os.path.normpath(
		os.path.join(os.getcwd(), "resources", "empty.package"))
	directory = os.path.dirname(filenames[0])
	new_package_path = os.path.normpath(os.path.join(directory, savefile))

	shutil.copyfile(empty_package_path, new_package_path)

	return c_lib.txtrExtractProcess(file_array, len(filenames), new_package_path.encode("utf-8"))
	