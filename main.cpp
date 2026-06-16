#include <iostream>
#include <map>

int main()
{
    std::map<std::string, int> m;

    // m["key"];

    if (m.find("key") != m.end())
        std::cout << "enter here" << std::endl;
}