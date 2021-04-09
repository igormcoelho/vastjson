
#include <bigjson/BigJSON.hpp>

#include <iostream>

int main()
{
    std::ifstream if_test("demo/test.json");
    bigjson::BigJSON bigj(if_test);
    std::cout << "LOADED #KEYS = " << bigj.size() << std::endl;

    return 0;
}