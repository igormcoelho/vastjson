#include <iostream>
#include <limits> // numeric_limits

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main()
#include <catch2/catch.hpp>

// Some instructions for Catch2
// https://github.com/catchorg/Catch2/blob/master/docs/test-cases-and-sections.md

#include <vastjson/VastJSON.hpp> // 'src' included

using namespace std;
using namespace vastjson;

const std::string example = "{\"A\":{ },\"B\":{ \"B1\":10, \"B2\":\"abcd\" },\"Z\":{ }}";

TEST_CASE("example size == 3")
{
    std::cout << "" << std::endl;
    std::string local_example = example;
    VastJSON bigj(local_example);
    REQUIRE(bigj.size() == 3);
}

TEST_CASE("bigj[\"B\"][\"B2\"] == \"abcd\"")
{
    std::cout << "" << std::endl;
    std::string local_example = example;
    VastJSON bigj(local_example);
    REQUIRE(bigj["B"]["B2"] == "abcd");
}

TEST_CASE("bigj[\"B\"][\"B1\"] == 10")
{
    std::cout << "" << std::endl;
    std::string local_example = example;
    VastJSON bigj(local_example);
    REQUIRE(bigj["B"]["B1"] == 10);
}

TEST_CASE("bigj isPending()")
{
    std::cout << "" << std::endl;
    // lazy processing
    VastJSON bigj1{new std::ifstream("testdata/test2.json")};
    REQUIRE(bigj1.isPending());
    // empty json (immediate processing)
    std::string str = "{}";
    VastJSON bigj2{str};
    REQUIRE(!bigj2.isPending());
}

TEST_CASE("bigj partial consumption over ifstream")
{
    std::cout << std::endl << " ======= partial consumption of ifstream ======= " << std::endl;
    std::unique_ptr<std::ifstream> ifs{new std::ifstream("testdata/test2.json")};
    VastJSON bigj{std::move(ifs)};

    // stream must exist
    REQUIRE(bigj.isPending());
    // key B and subkey B1 must be found
    REQUIRE(bigj["B"]["B1"] == 10);
    // stream must exist
    REQUIRE(bigj.isPending());
    // size must be correct
    REQUIRE(bigj.size() == 3);
    // stream must not exist (due to size() consumption)
    REQUIRE(!bigj.isPending());
}

TEST_CASE("bigj getUntil")
{
    std::cout << std::endl << " ======= getUntil ======= " << std::endl;
    std::unique_ptr<std::ifstream> ifs{new std::ifstream("testdata/test2.json")};
    VastJSON bigj{std::move(ifs)};

    // stream must exist
    REQUIRE(bigj.isPending());
    // cache size must be zero
    REQUIRE(bigj.cacheSize() == 0);
    // get one element
    bigj.getUntil("", 1);
    // cache size must be one
    REQUIRE(bigj.cacheSize() == 1);
    // get until "B" is found
    bigj.getUntil("B");
    // cache size must be two
    REQUIRE(bigj.cacheSize() == 2);
    // stream must exist
    REQUIRE(bigj.isPending());
    // get unlimited (empty targetKey and no count_keys)
    bigj.getUntil();
    // cache size must be three
    REQUIRE(bigj.cacheSize() == 3);
    // stream must not exist
    REQUIRE(!bigj.isPending());
}

TEST_CASE("bigj test_with_list")
{
    std::cout << "" << std::endl;
    std::unique_ptr<std::ifstream> ifs{new std::ifstream("testdata/test_with_list.json")};
    VastJSON bigj{std::move(ifs)};

    // size is correct
    REQUIRE(bigj.size() == 4);
}


TEST_CASE("bigj test_quotes")
{
    std::cout << "" << std::endl;
    std::unique_ptr<std::ifstream> ifs{new std::ifstream("testdata/test_quotes.json")};
    VastJSON bigj{std::move(ifs)};

    // size is correct
    REQUIRE(bigj.size() == 3);
}


TEST_CASE("bigj test primitive")
{
    std::cout << "" << std::endl;
    std::string s = "10";
    VastJSON bigj{s};

    // size is correct
    REQUIRE(bigj.size() == 1);
    // empty key means "self-value" or "root-value" or "primitive-value"
    // TODO: should we use some "operator*" or "self-conversion operator"?
    REQUIRE(bigj[""] == 10);

    std::string s2 = "[1,2,3]";
    VastJSON bigj2{s2};

    // size is correct
    REQUIRE(bigj2.size() == 1);
    // empty key means "self-value" or "root-value" or "primitive-value"
    // TODO: should we use some "operator*" or "self-conversion operator"?
    REQUIRE(bigj2[""].size() == 3);
}



TEST_CASE("bigj test_common")
{
    std::cout << "" << std::endl;
    std::unique_ptr<std::ifstream> ifs{new std::ifstream("testdata/test_common.json")};
    VastJSON bigj{std::move(ifs)};

    // size is correct
    REQUIRE(bigj.size() == 3);
}



TEST_CASE("bigj test local")
{
    std::cout << "" << std::endl;

    std::string  tst = "{\"A\":1, \"B\":true}";
    VastJSON bigj0{tst};

    // size is correct
    REQUIRE(bigj0.size() == 2);


    tst = "{\"A\":1,\"B\":true,\"C\":[1,2,3],\"D\":{\"D1\":10}, \"E\":-1}";
    VastJSON bigj{tst};

    // size is correct
    REQUIRE(bigj.size() == 5);


    tst = "{\"A\":1,\"B\":true,\"C\":[1,2,3],\"D\":{\"D1\":10}, \"E\":-1}";
    VastJSON bigj2{tst};

    // size is correct
    REQUIRE(bigj2.size() == 5);

    std::string test_base = "{\"A\":{},\"B\":{},\"C\":[{\"C1\": {}}, {\"C2\": {}}],\"D\":{\"D1\":{}}, \"E\":{}}";
    tst = test_base;
    VastJSON bigj3{tst, BIG_ROOT_DICT_NO_ROOT_LIST};

    // size is INcorrect (SHOULD BE 5!!)
    REQUIRE(bigj3.size() == 3);
    // mode should not allow lists on top (TODO: maybe flag some error() internal field in this situation?)
    REQUIRE(bigj3.getMode() == BIG_ROOT_DICT_NO_ROOT_LIST);

    tst = test_base;
    VastJSON bigj4{tst, BIG_ROOT_DICT_GENERIC};

    for (auto it = bigj4.begin(); it != bigj4.end(); it++)
        std::cout << "bigj4:" << it->first << std::endl;
    // size is Correct
    REQUIRE(bigj4.size() == 5);
    // mode should not allow lists on top (TODO: maybe flag some error() internal field in this situation?)
    REQUIRE(bigj4.getMode() == BIG_ROOT_DICT_GENERIC);
}
