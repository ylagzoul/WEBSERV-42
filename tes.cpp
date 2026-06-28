#include "server/server.hpp"





int main()
{
    std::string chunkNum = "4F";
    char *str;

    int chunkSize = std::strtol(chunkNum.c_str(), &str, 16);

    if (errno == ERANGE || str)
    cout << str << std::endl;
    cout << chunkSize << std::endl;
}