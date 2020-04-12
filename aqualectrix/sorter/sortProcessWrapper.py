import ctypes
import pathlib

# Load shared library into ctypes
c_lib = ctypes.CDLL("./libSortProcess.so")

# Provide details about sortProcess
# bool sortProcess (const char* filename, const int sortindex)
c_lib.sortProcess.restype = ctypes.c_bool
c_lib.sortProcess.argtypes = [ctypes.c_char_p, ctypes.c_int];

def sortindexFile(filename, index):
    return c_lib.sortProcess(filename.encode('utf-8'), index);
