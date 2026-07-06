#include "Record.hpp"

Record::Record()
{
    number = 0;
    fileFd = -1;
    fd = -1;
    dir = NULL;
    status = NEW_RS;
    method = NO_MD;
    clientState = DISCONNECTED;

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
