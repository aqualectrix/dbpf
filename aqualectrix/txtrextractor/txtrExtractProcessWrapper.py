import ctypes
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

	return c_lib.txtrExtractProcess(file_array, len(filenames), savefile.encode("utf-8"))
	