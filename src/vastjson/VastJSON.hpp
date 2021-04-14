// This code is part of VastJSON library: parsers for giant json files 
// It makes heavy usage of <nlohmann/json.hpp> library (also MIT licensed)
// Project website: https://github.com/igormcoelho/vastjson
// author: Igor Machado Coelho
// Copyleft 2021 - MIT License

#ifndef VAST_JSON_HPP
#define VAST_JSON_HPP

#ifdef INCLUDE_NLOHMANN_JSON_HPP_
#error VastJSON must be included before nlohmann::json. See https://github.com/igormcoelho/vastjson/issues/4
#endif

#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
//
#include <iostream> // TODO REMOVE
//
#include <memory>

namespace nlohmann
{
    namespace detail
    {

        template <class T>
        class custom_stream_adapter
        {
        public:
            using char_type = typename T::char_type;
            std::shared_ptr<T> adapter;

            ~custom_stream_adapter()
            {
                adapter = nullptr;
            }

            explicit custom_stream_adapter(std::shared_ptr<T> _adapter)
                : adapter(_adapter)
            {
            }

            // delete because of pointer members
            custom_stream_adapter(const custom_stream_adapter &) = delete;
            custom_stream_adapter &operator=(custom_stream_adapter &) = delete;
            custom_stream_adapter &operator=(custom_stream_adapter &&) = delete;

            custom_stream_adapter(custom_stream_adapter &&rhs) noexcept
                : adapter(rhs.adapter)
            {
                rhs.adapter = nullptr;
            }

            // std::istream/std::streambuf use std::char_traits<char>::to_int_type, to
            // ensure that std::char_traits<char>::eof() and the character 0xFF do not
            // end up as the same value, eg. 0xFFFFFFFF.
            std::char_traits<char>::int_type get_character()
            {
                return adapter->get_character();
            }
        };

        template <class T>
        inline custom_stream_adapter<T> input_adapter(std::shared_ptr<T> &stream_ptr)
        {
            return custom_stream_adapter<T>(stream_ptr);
        }

    } // namespace
} // namespace

//
#include <nlohmann/json.hpp>
//using json = nlohmann::json;

namespace vastjson
{
    // stream that caches stuff read by nlohmann::json
    struct CacheStream
    {
        using char_type = char;
        std::istream *is;
        std::string cache;

        CacheStream(std::istream *_is) : is(_is)
        {
        }

        std::char_traits<char>::int_type get_character()
        {
            int c = is->get();
            cache += c;
            return c;
        }
    };

    enum ModeVastJSON
    {
        // strategy for big dictionaries/objects on root level
        BIG_ROOT_DICT_GENERIC,
        // strategy for big dictionaries/objects on root level (do not allow lists as top-level entries)
        BIG_ROOT_DICT_NO_ROOT_LIST,
        // BIG_ROOT_LIST (TODO),
        // BIG_TARGET_ELEMENT (TODO),
        // NOT_BIG (TODO)
    };

    class VastJSON final
    {
    private:
        ModeVastJSON mode;
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
        // storing error flag here
        bool hasError = false;
        //
        ModeVastJSON getMode()
        {
            return mode;
        }

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
        VastJSON(std::string &str, ModeVastJSON _mode = ModeVastJSON::BIG_ROOT_DICT_GENERIC) : mode{_mode}
        {
            std::istringstream is(str);
            int count_par = 0; // reading from level 0
            cacheUntil(is, count_par);
        }

        // istream will be immediately processed (for top-level items)
        VastJSON(std::istream &is, ModeVastJSON _mode = ModeVastJSON::BIG_ROOT_DICT_GENERIC) : mode{_mode}
        {
            int count_par = 0; // reading from level 0
            cacheUntil(is, count_par);
        }

        // lazy processing
        VastJSON(std::unique_ptr<std::ifstream> &&_ifsptr, ModeVastJSON _mode = ModeVastJSON::BIG_ROOT_DICT_GENERIC)
            : mode{_mode}, ifsptr{std::move(_ifsptr)}
        {
        }

        // lazy processing
        VastJSON(std::ifstream &&_if, ModeVastJSON _mode = ModeVastJSON::BIG_ROOT_DICT_GENERIC)
            : mode{_mode}, ifsptr{new std::ifstream{std::move(_if)}}
        {
        }

        // lazy processing: transfer ownership of _if to VastJSON
        VastJSON(std::ifstream *_if, ModeVastJSON _mode = ModeVastJSON::BIG_ROOT_DICT_GENERIC)
            : mode{_mode}, ifsptr{_if}
        {
        }

        ~VastJSON()
        {
        }

    private:
        std::string getStringIdentifier(std::string &before)
        {
            //std::cout << "getStringIdentifier('" << before << "')" << std::endl;
            unsigned keyStart = before.find('\"') + 1;
            std::string rest = before.substr(keyStart, before.length());
            //std::cout << "rest = '" << rest << "'" << std::endl;
            std::istringstream is(rest);
            std::string str = "\"";
            getString(str, is);
            //std::cout << "final string: '" << str << "'" << std::endl;
            return str;
        }

        void getString(std::string &str, std::istream &is)
        {
            char c;
            while (is.get(c))
            {
                str += c;
                if (c == '\"') // closing string
                    break;
                if (c == '\\')
                {              // escape char, read next
                    is.get(c); // TODO: check if error is possible here
                    str += c;  // add directly
                }
            }
            if ((str.length() < 2) || (str[0] != '\"') || (str[str.length() - 1] != '\"'))
            {
                std::cout << "WARNING: BAD STRING '" << str << "'" << std::endl; // TODO: remove
                str = "\"\"";                                                    // forced empty string
            }
        }
        //
        nlohmann::json getJSONElement(std::istream &is)
        {
            char last = '\0';
            auto sptr = std::make_shared<CacheStream>(CacheStream(&is));
            nlohmann::json jj6;
            std::string again;
            bool bad = false;
            try
            {
                jj6 = nlohmann::json::parse(sptr);
            }
            catch (std::exception &e)
            {
                again = sptr->cache;
                last = again[again.length() - 1];
                again.pop_back();
                bad = true;
            }
            //
            if (bad)
            {
                try
                {
                    jj6 = nlohmann::json::parse(again);
                }
                catch (std::exception &e)
                {
                    //std::cout << "REALLY BAD READ! NO TRY AGAIN..." << std::endl;
                }
            }
            //
            //std::cout << "LAST(PUTBACK) = '" << last << "'" << std::endl;
            is.putback(last);
            return jj6;
        }
        //
        //void trim(std::istream& is, char& pk) {
        void trim(std::istream &is)
        {
            char pk = is.peek();
            //std::cout << "trim(pk='" << pk << "')" << std::endl;
            if (!pk)
                return; // EOF
            // consume spaces and non-visible chars
            while ((pk == ' ') || (pk == '\t') || (pk == '\r') || (pk == '\n')) // TODO: more here?
            {
                int gc = is.get();
                //std::cout << "gc='" << gc<< "' pk='" << pk << "'" << std::endl;
                //assert(gc == pk); // OBVIOUS TRUTH!
                pk = is.peek();
                if (!pk)
                    return; // EOF
            }
            //
        }

        // perform string caching until 'targetKey' is found (or stream is ended)
        void cacheUntil(std::istream &is, int &count_par, std::string targetKey = "", int count_keys = -1)
        {
            if (mode == ModeVastJSON::BIG_ROOT_DICT_NO_ROOT_LIST)
                cacheUntilNoRootList(is, count_par, targetKey, count_keys);
            else // ModeVastJSON::BIG_ROOT_DICT_GENERIC
                cacheUntilGeneric(is, count_par, targetKey, count_keys);
        }

        // IMPLEMENTATION THAT ALLOWS GENERIC JSON (SLOWER...)
        void cacheUntilGeneric(std::istream &is, int &count_par, std::string targetKey, int count_keys)
        {
            std::string before;
            std::string content;
            //
            // DETECT MODE
            // MODE 1 - { or }
            // MODE 2 - general (int, string, list)
            // TODO: MODE 3 - [ ]... but not now!
            //

            char pk = is.peek();
            //trim(is, pk);
            trim(is);
            pk = is.peek();

            // try to detect mode 2 (should not be '{' or continuation char ',')
            if ((pk != '{') && (pk != ','))
            {
                // must be a list or primary element
                nlohmann::json jout = getJSONElement(is);
                jsons[""] = jout;
                cache[""] = "";
                return;
            }

            // this is mode 1: first symbol must be '{'
            while (true)
            {
                // get part
                char c;
                if (!is.get(c))
                    break; // EOF

                // LOOK FOR IDENTIFIER
                if (c != '\"')
                {
                    //std::cerr << "WHAT TO DO? MAYBE TRIM? c='" << c << "'" << std::endl;
                    // could also be first '{' here... or final '}'...
                    //
                    //std::cout << "must treat c='" << c << "'" << std::endl;
                    // TODO: this may be useful for specific targetting of "huge object" or "huge list" modes...
                    //
                    trim(is);
                }
                else
                {
                    // directly load chain of string (including \", '{' and '}')
                    std::string str = "\"";
                    // invoke getString method
                    getString(str, is);
                    //
                    // try to get identifier
                    char pk = is.peek();
                    //trim(is, pk);
                    trim(is);
                    pk = is.peek();
                    if (pk != ':')
                    {
                        std::cerr << "WRONG DELIMITER! ABORT!" << std::endl;
                        break;
                    }
                    is.get(); // consume ':'
                    pk = is.peek();
                    //trim(is, pk);
                    trim(is);
                    pk = is.peek();
                    //
                    nlohmann::json comp = getJSONElement(is);

                    std::string str_id = getStringIdentifier(str);
                    std::string field_name = str_id.substr(1, str_id.length() - 2);
                    if (field_name == "")
                    {
                        std::cerr << "STRANGE: EMPTY ID!" << std::endl;
                        assert(false);
                    }
                    std::stringstream ss;
                    ss << comp;
                    comp = nlohmann::json();
                    //std::cout << "field_name: " << field_name << std::endl;
                    cache[field_name] = ss.str();
                    // TODO: delete 'ss' (AVOID LOSS OF MEMORY HERE)
                    content = ""; // implicit??
                    before = "";  // good?
                    // =============
                    // if 'targetKey' is found, stop reading
                    if ((targetKey != "") && (field_name == targetKey))
                    {
                        // perform count_par decrease and stop (for now)
                        //count_par--;
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
                            //count_par--;
                            break;
                        }
                    }

                    // =============
                    //continue;
                }
                //
                //std::cerr << "WHAT TO DO?" << std::endl;
            }
        }

        // LEGACY IMPLEMENTATION THAT WON'T ALLOW LISTS ON ROOT LEVEL... (FASTER!)
        void cacheUntilNoRootList(std::istream &is, int &count_par, std::string targetKey, int count_keys)
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
                        int keyStart = before.find('\"') + 1;
                        //std::string check_before1 = before.substr(0, keyStart);
                        //std::cout << "check_before1 = '" << check_before1 << "'" << std::endl;
                        int keyEnd = before.find('\"', keyStart + 1);
                        //std::string check_before2 = before.substr(keyEnd, before.length()-keyEnd);
                        //std::cout << "check_before2 = '" << check_before2 << "'" << std::endl;
                        //std::cout << "keyEnd = " << keyEnd << " keyStart = " << keyStart << std::endl;
                        std::string field_name = "";
                        if (keyEnd < 0)
                        {
                            std::cerr << "WARNING: VastJSON failed to get field (mode: BIG_ROOT_DICT_NO_ROOT_LIST)" << std::endl;
                            this->hasError = true;
                            before = ""; // manual remove
                        }
                        else
                        {
                            int keySize = keyEnd - keyStart;
                            field_name = before.substr(keyStart, keySize);
                            before = "";
                            //2-move string to cache
                            //std::cout << "x1 field_name: " << field_name << " content->" << content << std::endl;
                            cache[field_name] = std::move(content); // <------ IT'S FUNDAMENTAL TO std::move() HERE!
                        }
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
