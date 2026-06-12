#include <iostream>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
# define KB 1024

int main()
{
    // int fd = open("/home/abouhki/goinfre/dfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    // if (fd==-1)
    //     cout << "kayn chi error a wld 3ammi\n";
    // int i = 0;
    // string buff(10000, 'e');
    // while (i < KB * KB)
    // {
    //     write(fd, buff.c_str() , 10000);
    //     i++;
    // }
    size_t k = 18446744073709551617U;
    k+=2;
    cout << k;
}
