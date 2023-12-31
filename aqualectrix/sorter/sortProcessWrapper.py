import ctypes
import os
import pathlib

# Load shared library into ctypes
os.add_dll_directory("C:/msys64/ucrt64/bin")
os.add_dll_directory(os.getcwd())
c_lib = ctypes.CDLL("./libSortProcess.so")

# Provide details about sortProcess
# bool sortProcess (const char* filename, const int sortindex)
c_lib.sortProcess.restype = ctypes.c_bool
c_lib.sortProcess.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_bool];

def sortindexFile(filename, index, geneticize_hair):
    return c_lib.sortProcess(filename.encode('utf-8'), index, geneticize_hair);
