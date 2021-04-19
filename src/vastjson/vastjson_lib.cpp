
// import vastjson
#include "vastjson_lib.h"

// hpp VastJSON
#include<vastjson/VastJSON.hpp>

// initialize directly via string (good for testing)
void *
vastjson_init_string(const char *vr, int sz_vr)
{
    std::string str(vr);
    return new vastjson::VastJSON(str);
}

// initialize directly via filename and mode
void *
vastjson_init_filename(const char *vr, int sz_vr, int mode)
{
    std::string filename(vr);
    std::unique_ptr<std::ifstream> ifile { new std::ifstream(filename) };
    vastjson::ModeVastJSON mmode = (vastjson::ModeVastJSON)mode;
    return new vastjson::VastJSON(std::move(ifile), mmode);
}

// initialize directly via filename and mode
void
vastjson_destroy(void *obj)
{
    vastjson::VastJSON* vobj = (vastjson::VastJSON*) obj;
    delete vobj;
}

// ================
// auxiliar methods
// ================


void
vastjson_cache_until(void *obj, const char *targetKey, int sz_vr, int count_keys)
{
    vastjson::VastJSON* vobj = (vastjson::VastJSON*) obj;
    std::string key(targetKey);
    // this line will crash if getIfsptr fails
    vobj->cacheUntil(vobj->getIfsptr(), vobj->getCountParIfsptr(), key, count_keys);
}
//void cacheUntil(std::istream &is, int &count_par, std::string targetKey = "", int count_keys = -1)

void
vastjson_unload(void *obj, const char *targetKey, int sz_vr)
{

}
//void unload(std::string key)

char *
vastjson_at_cache(void *obj, const char *targetKey, int sz_vr)
{
    vastjson::VastJSON* vobj = (vastjson::VastJSON*) obj;
    std::string key(targetKey);
    std::string& cached = vobj->atCache(key);
    char *cstr = new char[cached.length() + 1];
    strcpy(cstr, cached.c_str());
    return cstr;
}
//gets string from cache

void
vastjson_free_string_ptr(char *str)
{
    delete str;
}
//frees string

extern "C" int
vastjson_size(void *obj)
{
    vastjson::VastJSON* vobj = (vastjson::VastJSON*) obj;
    return vobj->size();
}

extern "C" int
vastjson_cache_size(void *obj)
{
    vastjson::VastJSON* vobj = (vastjson::VastJSON*) obj;
    return vobj->cacheSize();
}

extern "C" void
vastjson_get_until(void *obj, const char *targetKey, int sz_vr, int count_keys)
{
    vastjson::VastJSON* vobj = (vastjson::VastJSON*) obj;
    std::string key(targetKey);
    vobj->getUntil(key, count_keys);
}
