#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        std::cerr << "fork failed\n";
        return 1;
    }

    if (pid == 0)
    {
        // Child process
        char *argv[] = {
            (char *)"python3",
            (char *)"test.py",
            NULL
        };

        execv("/usr/bin/python3", argv);

        // exec only returns if it fails
        perror("execl");
        _exit(127);
    }

    // Parent process
    int status;

    pid_t ret = waitpid(pid, &status, 0);

    if (ret == -1)
    {
        perror("waitpid");
        return 1;
    }

    std::cout << "Raw status: " << status << std::endl;

    if (WIFEXITED(status))
    {
        std::cout << "Child exited normally\n";
        std::cout << "Exit code: " << WEXITSTATUS(status) << std::endl;
    }
    else if (WIFSIGNALED(status))
    {
        std::cout << "Child terminated by signal "
                  << WTERMSIG(status) << std::endl;
    }
    else if (WIFSTOPPED(status))
    {
        std::cout << "Child stopped by signal "
                  << WSTOPSIG(status) << std::endl;
    }

    return 0;
}   