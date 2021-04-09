#ifndef VAST_JSON_HPP
#define VAST_JSON_HPP

#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
//
#include <nlohmann/json.hpp>
//using json = nlohmann::json;

namespace vastjson
{

    class VastJSON
    {
    public:
        // multiple json
        std::map<std::string, nlohmann::json> jsons;
        // read string cache
        std::map<std::string, std::string> cache;
        // pending reads
        std::shared_ptr<std::ifstream> ifsptr;
        // count delimiters {} for ifsptr
        // this variable was local, now it's global since stream consumption can be continued over ifsptr
        int count_par_ifsptr = 0;

        unsigned size()
        {
            if (ifsptr)
            {
                // must cache all available entries (to calculate 'size()')
                cacheUntil(*ifsptr, count_par_ifsptr, "");
                // stream has been consumed, drop its memory pointer
                ifsptr = std::shared_ptr<std::ifstream>();
            }

            return this->cache.size();
        }

        nlohmann::json &operator[](std::string key)
        {
            return this->getKey(key);
        }

        // get key in json structured format
        nlohmann::json &getKey(std::string key)
        {
            auto it = jsons.find(key);
            if (it != jsons.end())
            {
                return jsons[key];
            }
            auto it2 = cache.find(key);
            if (it2 == cache.end())
            {
                // CHECK IF THERE'S MORE TO READ IN 'ifsptr'
                if (ifsptr)
                {
                    // must cache all available entries (to calculate 'size()')
                    cacheUntil(*ifsptr, count_par_ifsptr, key);
                    // IF stream has been consumed, drop its memory pointer
                    if (ifsptr->eof())
                        ifsptr = std::shared_ptr<std::ifstream>();
                    // try again and update iterator
                    it2 = cache.find(key);
                    if (it2 == cache.end())
                    {
                        // NOTHING ELSE TO DO... KEY DOES NOT EXIST!
                        //
                        //std::cerr << "BigJSON::getKey() error: key '" << key << "' does not exist!" << std::endl;
                    }
                }
            }
            if (cache[key] == "")
            {
                //std::cerr << "BigJSON::getKey() error: key '" << key << "' is empty or has been unloaded!" << std::endl;
            }

            // TODO: continue even with error (or return 'optional' for recovery?)
            jsons[key] = nlohmann::json::parse(std::move(cache[key]));
            cache[key] = "";
            return jsons[key];
        }

        // unload json structure and do not keep string cache
        void unload(std::string key)
        {
            cache[key] = ""; // mark as empty
            auto it = jsons.find(key);
            if (it == jsons.end())
            {
                //std::cerr << "BigJSON::unload() error: json key '" << key << "' does not exist!" << std::endl;
            }
            jsons.erase(it); // drop json structure
        }

        // move json structure back to string cache (since json structured format may be more memory costly)
        void toCache(std::string key)
        {
            auto it = jsons.find(key);
            if (it == jsons.end())
            {
                //std::cerr << "BigJSON::toCache() error: json key '" << key << "' does not exist!" << std::endl;
            }
            //
            std::stringstream ssjson;
            ssjson << jsons[key];
            unload(key);               // unload json structure
            cache[key] = ssjson.str(); // keep string in cache
        }

        VastJSON(std::string &str)
        {
            std::istringstream is(str);
            int count_par = 0; // reading from level 0
            cacheUntil(is, count_par, "");
        }

        VastJSON(std::istream &is)
        {
            int count_par = 0; // reading from level 0
            cacheUntil(is, count_par, "");
        }

        VastJSON(std::shared_ptr<std::ifstream> _ifsptr) : ifsptr{_ifsptr}
        {
        }

        ~VastJSON()
        {
        }

    private:
        //
        // perform string caching until 'targetKey' is found (or stream is ended)
        void cacheUntil(std::istream &is, int &count_par, std::string targetKey = "")
        {
            std::string before;
            std::string content;
            //
            int target_field = 1; // starts from 1
            bool presave = true;
            bool save = false;
            //
            while (true)
            {
                char c;
                if (!is.get(c))
                    break; // EOF
                if (presave)
                    before += c;
                if (save)
                    content += c;
                if (c == '{')
                {
                    count_par++;
                    if ((count_par == target_field + 1) && presave) // 2?
                    {
                        save = true;
                        content += c;
                        presave = false;
                    }
                }
                if (c == '}')
                {
                    if ((count_par == target_field + 1) && save) // 2?
                    {
                        //
                        //std::cout << "RESTART = " << sbefore << std::endl;
                        //
                        // 1-get field name
                        unsigned keyStart = before.find('\"') + 1;
                        unsigned keySize = before.find('\"', keyStart + 1) - keyStart;
                        std::string field_name = before.substr(keyStart, keySize);
                        before = "";
                        //2-move string to cache
                        cache[field_name] = std::move(content); // <------ IT'S FUNDAMENTAL TO std::move() HERE!
                        //
                        //std::cout << "store = '" << field_name << "'" << std::endl;
                        //
                        //before = std::stringstream();
                        //content = std::stringstream();
                        content = "";
                        //
                        presave = true;
                        save = false;
                        // if 'targetKey' is found, stop reading
                        if ((targetKey != "") && (field_name == targetKey))
                        {
                            // perform count_par decrease and stop (for now)
                            count_par--;
                            break;
                        }
                    }
                    count_par--;
                }
            }
        }

    public:
    };

} // namespace vastjson

#endif // VAST_JSON_HPP