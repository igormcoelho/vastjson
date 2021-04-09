
#include <vastjson/VastJSON.hpp>

#include <iostream>

int main()
{
    std::ifstream if_test("demo/test.json");
    vastjson::VastJSON bigj(if_test);
    std::cout << "LOADED #KEYS = " << bigj.size() << std::endl;

    std::ifstream if_test2("demo/test2.json");
    vastjson::VastJSON bigj2(if_test2);
    std::cout << "LOADED #KEYS = " << bigj2.size() << std::endl; // 3
    std::cout << bigj2["A"] << std::endl;
    std::cout << bigj2["B"]["B2"] << std::endl;
    std::cout << bigj2["Z"] << std::endl;

    std::cout << "====== tests with lazy loading file ======" << std::endl;

    vastjson::VastJSON bigj3(new std::ifstream("demo/test3.json"));
    // pending operations
    std::cout << "isPending(): " << bigj3.isPending() << std::endl;
    // cache size
    std::cout << "cacheSize(): " << bigj3.cacheSize() << std::endl;
    std::cout << "getUntil(\"\",1) first key is found" << std::endl;
    // get first keys
    bigj3.getUntil("", 1);
    // iterate over top-level keys (cached only!)
    for (auto it = bigj3.begin(); it != bigj3.end(); it++)
        std::cout << it->first << std::endl;
    // direct access will load more
    std::cout << "direct access to bigj3[\"B\"][\"B1\"] = " << bigj3["B"]["B1"] << std::endl;
    // cache size
    std::cout << "cacheSize(): " << bigj3.cacheSize() << std::endl;    
    // still pending
    std::cout << "isPending(): " << bigj3.isPending() << std::endl;
    // iterate over top-level keys (cached only!)
    for (auto it = bigj3.begin(); it != bigj3.end(); it++)
        std::cout << it->first << std::endl;

    // real size (will force performing top-level indexing)
    std::cout << "compute size will force top-level indexing...\nsize(): " << bigj3.size() << std::endl;
    // cache size
    std::cout << "cacheSize(): " << bigj3.cacheSize() << std::endl;    
    // not pending anymore
    std::cout << "isPending(): " << bigj3.isPending() << std::endl;
    // iterate over top-level keys (cached only!)
    for (auto it = bigj3.begin(); it != bigj3.end(); it++)
        std::cout << it->first << std::endl;

    return 0;
}
