#include "Record.hpp"

Record::Record()
{
    number = 0;
    fileFd = -1;
    fd = -1;
    dir = NULL;
    status = NEW_RS;
    method = NO_MD;
}

// Record::~Record()
// {
//     if (fileFd != -1)
//         close(fileFd);
//     if (fd != -1)
//         close(fd);
//     if (dir)
//         closedir(dir);
// }