import ctypes
import os
import pathlib

# Load shared library into ctypes
os.add_dll_directory("C:/msys64/ucrt64/bin")
os.add_dll_directory(os.getcwd())
c_lib = ctypes.CDLL("./libGetTexIdProcess.so")

# Provide details about getTexIdProcess
# const char* getTexIdProcess (const char* filename)
c_lib.getTexIdProcess.restype = ctypes.c_char_p
c_lib.getTexIdProcess.argtypes = [ctypes.c_char_p];

def getTexId(filename):
    return c_lib.getTexIdProcess(filename.encode('utf-8'));
