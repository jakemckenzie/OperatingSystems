#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>// The book forgot to include
#include <sys/wait.h>// these libraries? *tsk tsk*
int main() {
    pid_t pid; /* fork a child process */
    pid = fork();
    if (pid < 0) { /* error occurred */
        fprintf(stderr, "Fork Failed");
        return 1;
    } else if (pid == 0) { /* child process */
        execlp("/bin/ls","ls",NULL);
        printf("LINE J\n\n");
    } else { /* parent process */
        /* parent will wait for the child to complete */
        wait(NULL);
        printf("Child Complete\n\n");
    }
    return 0;
}