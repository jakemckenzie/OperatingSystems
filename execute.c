#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
    /* I felt as though the example included was a bit too complicated 
     * for what we wanted to accomplish so I wrote my own. 
     * 
     * Jake McKenzie Operating Systems extra credit.
     *
     * October 20, 2018.
     * 
     * This program forks and executes a program specified by my instructor.
     */

int main (int argc, char *argv[]) {
    pid_t c_pid = fork();
    int status;
    if (c_pid < 0) {
        perror("There was an error with the fork.\n\n");
        exit(EXIT_FAILURE);
    } else if (c_pid == 0) {
        printf("The PID for the process is is %ld\n\n", (long) getpid());
        exit(argc != 2 ? 0 : atoi(argv[1]));
    } else {
        printf("The child PID for the process is is %ld\n\n", (long) c_pid);
        wait(&status);
        printf("The process %ld exited with the status %ld\n\n", (long) c_pid, (long) status);
        exit(0);
    }
}