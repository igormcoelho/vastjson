#!/usr/bin/python3

import ctypes
#from functools import total_ordering
from typing import TypeVar, Type
T = TypeVar('T', bound='VastJSON')

vastjson_lib = ctypes.cdll.LoadLibrary('src/vastjson_py/cpp-build/libvastjson.so')
# csbiginteger_add(byte* big1, int sz_big1, byte* big2, int sz_big2, byte* vr, int sz_vr) -> int
#
#vastjson_init_string(const char *vr, int sz_vr) -> void *
vastjson_lib.vastjson_init_string.argtypes = [
    ctypes.c_char_p, ctypes.c_int] # ctypes.c_void_p
vastjson_lib.vastjson_init_string.restype = ctypes.c_void_p

#vastjson_init_filename(const char *vr, int sz_vr, int mode) -> void *
vastjson_lib.vastjson_init_filename.argtypes = [
    ctypes.c_char_p, ctypes.c_int, ctypes.c_int] # ctypes.c_void_p
vastjson_lib.vastjson_init_filename.restype = ctypes.c_void_p

#vastjson_destroy(void *obj) -> void
vastjson_lib.vastjson_destroy.argtypes = [
    ctypes.c_void_p] # void

#vastjson_cache_until(void *obj, const char *targetKey, int sz_vr, int count_keys) -> void
vastjson_lib.vastjson_cache_until.argtypes = [
    ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int, ctypes.c_int] # void

#vastjson_unload(void *obj, const char *targetKey, int sz_vr) -> void
vastjson_lib.vastjson_unload.argtypes = [
    ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int] # void

#vastjson_at_cache(void *obj, const char *targetKey, int sz_vr) -> char *
vastjson_lib.vastjson_at_cache.argtypes = [
    ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int] # void
vastjson_lib.vastjson_at_cache.restype = ctypes.c_char_p

#vastjson_free_string_ptr(char *str) -> void
vastjson_lib.vastjson_free_string_ptr.argtypes = [
    ctypes.c_char_p] # void

# size
vastjson_lib.vastjson_size.argtypes = [
    ctypes.c_void_p]
vastjson_lib.vastjson_size.restype = ctypes.c_int

# cacheSize
vastjson_lib.vastjson_cache_size.argtypes = [
    ctypes.c_void_p]
vastjson_lib.vastjson_cache_size.restype = ctypes.c_int


# ===============================================================
# VastJSON (py) is a pythonic API to VastJSON C++ implementation
# ===============================================================

class VastJSON(object):
    def __init__(self, param : str, mode: int = -1) -> T:
        #print("INIT")
        #if type(param) is str:
        if mode == -1:
            # direct string
            strsize = len(param)
            strdata = (ctypes.c_char * strsize).from_buffer(bytearray(param, 'ascii'))

            self._vjptr = vastjson_lib.vastjson_init_string(strdata, strsize)
            if self._vjptr == 0:
                raise ValueError('Error! No VastJSON C++ Pointer returned')
        # more options here?

    def __del__(self):
        #print("EXIT")
        vastjson_lib.vastjson_destroy(self._vjptr)
        self._vjptr = 0 # IS THIS NECESSARY?

    def atCache(self, param: str):
        print("AT CACHE")
        strsize = len(param)
        strdata = (ctypes.c_char * strsize).from_buffer(bytearray(param, 'ascii'))
        charptr = vastjson_lib.vastjson_at_cache(self._vjptr, strdata, strsize)
        charptr2 = ctypes.cast( charptr, ctypes.c_char_p )
        local_str = charptr2.value
        # free string (IS THIS NECESSARY?)
        #vastjson_lib.vastjson_free_string_ptr(charptr)
        return local_str
    
    def size(self):
        return vastjson_lib.vastjson_size(self._vjptr)

    def cacheSize(self):
        return vastjson_lib.vastjson_cache_size(self._vjptr)
