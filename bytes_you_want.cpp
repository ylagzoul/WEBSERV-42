#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;
# define KB 1024

int main()
{
    int fds[2];
    pipe(fds);
    pid_t pid = fork();
    char    *a[] = {NULL};
    if (pid == 0)
    {
        // dup2(fds[0], 0);
        // close(fds[1]);
        close(0);
        execve("/home/ylagzoul/Desktop/vv/server/test.py", a, NULL);
    }
    close(fds[1]);
    close(fds[0]);
    waitpid(pid, NULL, 0);
}
