#ifndef VASTJSON_LIB_H
#define VASTJSON_LIB_H

#include <stdio.h>

// initialize directly via string (good for testing)
extern "C" void *
vastjson_init_string(const char *vr, int sz_vr);

// initialize directly via filename and mode
extern "C" void *
vastjson_init_filename(const char *vr, int sz_vr, int mode);

// initialize directly via filename and mode
extern "C" void
vastjson_destroy(void *obj);

// ================
// auxiliar methods
// ================

extern "C" void
vastjson_cache_until(void *obj, const char *targetKey, int sz_vr, int count_keys);
//void cacheUntil(std::istream &is, int &count_par, std::string targetKey = "", int count_keys = -1)

extern "C" void
vastjson_unload(void *obj, const char *targetKey, int sz_vr);
//void unload(std::string key)

extern "C" char *
vastjson_at_cache(void *obj, const char *targetKey, int sz_vr);
//gets string from cache

extern "C" void
vastjson_get_until(void *obj, const char *targetKey, int sz_vr, int count_keys);
//void getUntil(std::string targetKey = "", int count_keys = -1)

extern "C" void
vastjson_free_string_ptr(char *str);
//frees string

extern "C" int
vastjson_size(void *obj);
//gets size

extern "C" int
vastjson_cache_size(void *obj);
//gets size


#endif // VASTJSON_LIB_H