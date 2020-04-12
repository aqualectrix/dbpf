import ctypes
import pathlib

# Load shared library into ctypes
libname = pathlib.Path().absolute() / "libSortProcess.so"
c_lib = ctypes.CDLL(str(libname))

# Provide details about sortProcess
# bool sortProcess (const char* filename, const int sortindex)
c_lib.sortProcess.restype = ctypes.c_bool
c_lib.sortProcess.argtypes = [ctypes.c_char_p, ctypes.c_int];

def sortindexFile(filename, index):
    return c_lib.sortProcess(filename.encode('utf-8'), index);
