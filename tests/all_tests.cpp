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
    std::string local_example = example;
    VastJSON bigj(local_example);
    REQUIRE(bigj.size() == 3);
}

TEST_CASE("bigj[\"B\"][\"B2\"] == \"abcd\"")
{
    std::string local_example = example;
    VastJSON bigj(local_example);
    REQUIRE(bigj["B"]["B2"] == "abcd");
}

TEST_CASE("bigj[\"B\"][\"B1\"] == 10")
{
    std::string local_example = example;
    VastJSON bigj(local_example);
    REQUIRE(bigj["B"]["B1"] == 10);
}

TEST_CASE("bigj partial consumption over ifstream")
{
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
