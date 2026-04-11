#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>

int main()
{
    while (1)
    {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in serverAddress;
        socklen_t len;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(8080);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        len = sizeof(serverAddress);

        int bind_fd = bind(server_fd,(struct sockaddr*)&serverAddress, sizeof(serverAddress));

        int listen_fd = listen(server_fd, 5);

        int clent_fd = accept(server_fd, (struct sockaddr*)&serverAddress, &len);

        // char msg[] = "I'm fin";
        char buffer[1024] = {0};
        recv(clent_fd,buffer,1024,0);
        // mradoun_parsin_reqouste();
        
        // send(clent_fd,msg,strlen(msg),0);

        std::cout << "buffer = " << buffer << std::endl;
    }
}