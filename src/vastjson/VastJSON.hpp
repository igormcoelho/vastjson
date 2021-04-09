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
    class VastJSON final
    {
    private:
        // multiple json
        std::map<std::string, nlohmann::json> jsons;
        // read string cache
        std::map<std::string, std::string> cache;
        // pending reads
        std::unique_ptr<std::ifstream> ifsptr;
        // count delimiters {} for ifsptr
        // this variable was local, now it's global since stream consumption can be continued over ifsptr
        int count_par_ifsptr = 0;

    public:
        const auto begin() const
        {
            return cache.begin();
        }

        const auto end() const
        {
            return cache.end();
        }

        bool isPending() const
        {
            return ifsptr != nullptr;
        }

        // cached items size. Note that: cacheSize() <= size()
        unsigned cacheSize() const
        {
            return this->cache.size();
        }

        // return number of top-level entries (not REALLY const...)
        unsigned size() const
        {
            if (ifsptr)
            {
                // sorry, this is quite fake, but necessary!
                // I know what I'm doing!
                VastJSON *me = const_cast<VastJSON *>(this);
                //
                // must cache all available entries (to calculate 'size()')
                me->cacheUntil(*me->ifsptr, me->count_par_ifsptr);
                // stream has been consumed, drop its memory pointer
                me->ifsptr = std::unique_ptr<std::ifstream>();
            }

            return this->cache.size();
        }

        // public method: advance on stream until 'targetKey' key is found, or 'count_keys' keys are found
        void getUntil(std::string targetKey = "", int count_keys = -1)
        {
            if (ifsptr)
            {
                // must cache all available entries (to calculate 'size()')
                cacheUntil(*ifsptr, count_par_ifsptr, targetKey, count_keys);

                // IF stream has been consumed, drop its memory pointer
                if (ifsptr->eof())
                    ifsptr = std::unique_ptr<std::ifstream>();
            }
        }

        // gets key json
        nlohmann::json &operator[](std::string key)
        {
            return this->getKey(key);
        }

        // gets key json (not REALLY const...)
        const nlohmann::json &operator[](std::string key) const
        {
            return this->getKey(key);
        }

        // get key in json structured format (not REALLY const...)
        const nlohmann::json &getKey(std::string key) const
        {
            // sorry, this is quite fake, but necessary!
            // I know what I'm doing!
            VastJSON *me = const_cast<VastJSON *>(this);
            return me->getKey(key);
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
                        ifsptr = std::unique_ptr<std::ifstream>();
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
            cache[key] = ""; // mark as empty
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

        // ======================
        //     constructors
        // ======================

        // string will be immediately processed (for top-level items)
        VastJSON(std::string &str)
        {
            std::istringstream is(str);
            int count_par = 0; // reading from level 0
            cacheUntil(is, count_par);
        }

        // istream will be immediately processed (for top-level items)
        VastJSON(std::istream &is)
        {
            int count_par = 0; // reading from level 0
            cacheUntil(is, count_par);
        }

        // lazy processing
        VastJSON(std::unique_ptr<std::ifstream> &&_ifsptr) : ifsptr{std::move(_ifsptr)}
        {
        }

        // lazy processing
        VastJSON(std::ifstream &&_if) : ifsptr{new std::ifstream{std::move(_if)}}
        {
        }

        // lazy processing: transfer ownership of _if to VastJSON
        VastJSON(std::ifstream* _if) : ifsptr{_if}
        {
        }

        ~VastJSON()
        {
        }

    private:
        //
        // perform string caching until 'targetKey' is found (or stream is ended)
        void cacheUntil(std::istream &is, int &count_par, std::string targetKey = "", int count_keys = -1)
        {
            std::string before;
            std::string content;
            //
            int target_field = 1; // starts from 1
            bool save = false;
            //
            while (true)
            {
                char c;
                if (!is.get(c))
                    break; // EOF
                if (!save)
                    before += c;
                if (save)
                    content += c;
                if (c == '{')
                {
                    count_par++;
                    if ((count_par == target_field + 1) && !save) // 2?
                    {
                        content += c;
                        save = true;
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
                        save = false;
                        // if 'targetKey' is found, stop reading
                        if ((targetKey != "") && (field_name == targetKey))
                        {
                            // perform count_par decrease and stop (for now)
                            count_par--;
                            break;
                        }
                        // check if count_keys is enabled (>= 0)
                        if (count_keys >= 0)
                        {
                            // counting is enabled.. must decrease one key
                            count_keys--;
                            // check if count has been reached
                            if (count_keys == 0)
                            {
                                // perform count_par decrease and stop (for now)
                                count_par--;
                                break;
                            }
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
