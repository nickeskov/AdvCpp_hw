//#include <sys/types.h>
//#include <sys/wait.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <string.h>
//#include <errno.h>
//
//#define PIPE_WRITE (1)
//#define PIPE_READ (0)
//
//int main(int argc, char *argv[]) {
//    int pipefd_to_child[2];
//    int pipefd_from_child[2];
//
//    pid_t cpid;
//    char buf;
//
//    if (argc != 3) {
//        fprintf(stderr, "Usage: %s <executable> <executable input>\n", argv[0]);
//        exit(EXIT_FAILURE);
//    }
//
//    if (pipe(pipefd_to_child) == -1) {
//        perror("pipefd_to");
//        exit(EXIT_FAILURE);
//    }
//
//    if (pipe(pipefd_from_child) == -1) {
//        close(pipefd_to_child[PIPE_READ]);
//        close(pipefd_to_child[PIPE_WRITE]);
//        perror("pipefd_from");
//        exit(EXIT_FAILURE);
//    }
//
//    cpid = fork();
//    if (cpid == -1) {
//        perror("fork");
//        exit(EXIT_FAILURE);
//    }
//
//    if (cpid == 0) {
//        close(pipefd_to_child[PIPE_WRITE]);
//        close(pipefd_from_child[PIPE_READ]);
//
//        if (dup2(pipefd_to_child[PIPE_READ], STDIN_FILENO) == -1
//            || dup2(pipefd_from_child[PIPE_WRITE], STDOUT_FILENO) == -1) {
//
//            close(pipefd_to_child[PIPE_READ]);
//            close(pipefd_from_child[PIPE_WRITE]);
//
//            _exit(EXIT_FAILURE);
//        }
//
//        if (close(pipefd_to_child[PIPE_READ]) == -1) {
//            close(pipefd_from_child[PIPE_WRITE]);
//            _exit(EXIT_FAILURE);
//        }
//        if (close(pipefd_from_child[PIPE_WRITE]) == -1) {
//            close(pipefd_to_child[PIPE_READ]);
//            _exit(EXIT_FAILURE);
//        }
//
//        if (execl(argv[1], argv[1], NULL) == -1) {
//            fprintf(stderr, "failed to execute %s: %s\n", argv[1], strerror(errno));
//            _exit(EXIT_FAILURE);
//        }
//    } else {            /* Parent writes argv[1] to pipe */
//        close(pipefd_to_child[PIPE_READ]);
//        close(pipefd_from_child[PIPE_WRITE]);
//
//        size_t argv1_len = strlen(argv[2]);
//        write(pipefd_to_child[PIPE_WRITE], argv[2], argv1_len);
//        close(pipefd_to_child[PIPE_WRITE]);          /* Reader will see EOF */
//
//        while (read(pipefd_from_child[PIPE_READ], &buf, 1) > 0) {
//            write(STDOUT_FILENO, &buf, 1);
//        }
//
//        close(pipefd_from_child[PIPE_READ]);          /* Close read end */
//
//        wait(NULL);                /* Wait for child */
//        exit(EXIT_SUCCESS);
//    }
//}

#include "process.h"

int main() {
    auto proc = linuxproc::Process(linuxproc::Process("sdfsdf", ""));
}