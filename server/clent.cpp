#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>


int main ()
{
    // while (1)
    // {
        int clent_fd = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in clentAddr;
        clentAddr.sin_family = AF_INET;
        clentAddr.sin_port = htons(8080);
        clentAddr.sin_addr.s_addr = INADDR_ANY;

        int bind_fd = connect(clent_fd, (struct sockaddr*)&clentAddr, sizeof(clentAddr));

        char msg[] = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n";
        send(clent_fd,msg,strlen(msg),0);

        // char buffr[1024] = {0};
        
        // recv(clent_fd,buffr,1024,0);


        // std::cout << "buffr = "<< buffr << std::endl;
    //}
}