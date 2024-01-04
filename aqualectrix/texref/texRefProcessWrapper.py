import ctypes
import os
import pathlib

# Load shared library into ctypes
os.add_dll_directory("C:/msys64/ucrt64/bin")
os.add_dll_directory(os.getcwd())
c_lib = ctypes.CDLL("./libtexRefProcess.so")

# Provide details about texRefProcess
# bool texRefProcess (const char* filename, const char* tex_id, const char* subset_to_replace)
c_lib.texRefProcess.restype = ctypes.c_bool
c_lib.texRefProcess.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p];

def texRefFile(filename, tex_id, subset_to_replace):
    return c_lib.texRefProcess(filename.encode('utf-8'), tex_id, subset_to_replace.encode('utf-8'));
