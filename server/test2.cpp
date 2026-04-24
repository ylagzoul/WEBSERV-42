Server::Server()
{
    clientId = 0;
}


int main()
{
    Config conf;
    Server obj;

    std::vector<Record> recs;
    std::vector<Request> req;
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int n = 0; //
    
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    conf.config_file();
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3001);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    bind(server_fd, (sockaddr*)&addr, len);
    listen(server_fd, 5);
    
    std::vector<pollfd> fds;
    
    pollfd server_poll;
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;
    fds.push_back(server_poll);


    obj.data.push_back(Data());
    recs.push_back(Record());
    req.push_back(Request());

    while (true)
    {
        int ret = poll(fds.data(),fds.size(), -1);

        if (ret == -1)
        {
            std::cout << "poll error\n";
            continue;
        }
    
        for (size_t i = 0; i < fds.size(); )
        {
            std::cout << "fd=" << fds[i].fd
                << " revents=" << fds[i].revents << std::endl;
            if (fds[i].fd == server_fd && (fds[i].revents & POLLIN))
            {
                int client_fd = accept(server_fd, (sockaddr*)&addr, &len);
                if (client_fd != -1)
                {
                    pollfd client_poll;
                    client_poll.fd = client_fd;
                    client_poll.events = POLLIN;
                    fds.push_back(client_poll);

                    recs.push_back(Record());
                    req.push_back(Request());
                    obj.data.push_back(Data());
                }
                p.revents = 0;
                i++;
                continue;
            }

            if (fds[i].revents & POLLIN)
            {
                char buffer[1024] = {0};
                int bytes = recv(fds[i].fd, buffer, 1024, 0);
                if (bytes > 0)
                {
                    int index1 = i - 1;
                    obj.clientId = i;
                    readRequest(req, index1, buffer, bytes);
                    p.events |= POLLOUT;
                    
                }
                else
                {
                    close(fds[i].fd); 
                    fds.erase(fds.begin() + i);
                    continue ;
                }

            }
            if ((fds[i].revents & POLLOUT))
            {
                int index = i - 1;
                if (obj.data[i].sendNext == true)
                {
                    obj.clientId = i;
                    obj.data[i].response = respond(recs[index], obj, conf,req[index]);
                }

                // std::cout <<  obj.data[i].response << std::endl;

                n += send(fds[i].fd, obj.data[i].response.c_str(), obj.data[i].response.size(), 0);
                if (n > 0)
                {
                    int max = obj.data[i].response.size();
                    if (n < max)
                    {
                        obj.data[i].response = obj.data[i].response.substr(n);
                        obj.data[i].sendNext = false;
                    }
                    
                }
                else if (n == -1)
                {
                    obj.data[i].sendNext = true;
                }
                n = 0;
            }
            if (fds[i].revents & (POLLHUP | POLLERR))
            {
                close(fds[i].fd); 
                fds.erase(fds.begin() + i);
                continue;
            }
        
            
        }
    }
    close(server_fd);
}
