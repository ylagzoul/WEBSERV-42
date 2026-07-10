#include "server.hpp"

Record::Record()
{
    fileFd = -1;
    pi = false;
    fd = -1;
    dir = NULL;
    status = NEW_RS;
    method = NO_MD;
    clientState = DISCONNECTED;
    killProsses = false;

    sendNext = true;
    time = std::time(NULL);
}

void Record::clean()
{
    if (fileFd != -1)
        (close(fileFd), fileFd = -1);
    if (fd != -1)
        (close(fd), fd = -1);
    if (dir)
        (closedir(dir), dir = NULL);
}
