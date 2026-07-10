#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
    pid_t pid = fork();

    if (pid == 0)
    {
        printf("Child\n");
       return (1);
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
    {
        printf("Child exited normally\n");
        printf("Exit code = %d\n", WEXITSTATUS(status));
    }
}