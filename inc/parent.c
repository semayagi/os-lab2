/*
stdin → parent → pipe[0] → child1 → pipe[1] → child2 → pipe[2] → parent → stdout
        читает              UPPER              пробел→_
        строки              CASE


*/
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>   // getline, printf, stdin
#include <stdlib.h>  // free
#include <unistd.h>  // write, read, fork, dup2, close, execv
#include <sys/types.h> // pid_t, ssize_t

int main(void) {
    int pipes[3][2];
    pipe(pipes[0]); // создаёт pipes[0][0] (чтение) и pipes[0][1] (запись)
    pipe(pipes[1]);
    pipe(pipes[2]);

    pid_t child1_pid = fork();
    // Отныне код выполняется двумя процессами: родительский и дочерний
    // child1_pid = PID > 0 (в родительском)
    // child1_pid = 0 в дочернем и выполняется блок ниже:
    if (child1_pid == 0) { // если мы внутри дочернего процесса
    //  int dup2(int oldfd, int newfd) - переназначение файлового дескриптора
        dup2(pipes[0][0], STDIN_FILENO);    // читать из pipe[0], STDIN_FILENO = 0
        dup2(pipes[1][1], STDOUT_FILENO);   // писать в pipe[1],  STDOUT_FILENO = 1
        // Что сделали: теперь для child1 stdin - это то, что прилетает ему из pipe[0], а stdout выводится в pipe[1]

        for (int i = 0; i < 3; ++i) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        char *args[] = { "./child1", NULL };
        execv("./child1", args); // Далее код выполняется только родителем
    }

    pid_t child2_pid = fork();
    if (child2_pid == 0) { 
        dup2(pipes[1][0], STDIN_FILENO);   
        dup2(pipes[2][1], STDOUT_FILENO);  

        for (int i = 0; i < 3; ++i) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        char *args[] = { "./child2", NULL };
        execv("./child2", args);
    }

    // Закрываем ненужные дескрипторы
    close(pipes[0][0]);
    close(pipes[1][0]);
    close(pipes[1][1]);
    close(pipes[2][1]);

    char *line = NULL;
    size_t capacity = 0; // размер выделенного буфера
    ssize_t length;

    while((length = getline(&line, &capacity, stdin)) != -1) {
        write(pipes[0][1], line, (size_t)length);
        char buffer[4096];
        ssize_t received = read(pipes[2][0], buffer, sizeof(buffer) - 1);
        if (received > 0) {
            buffer[received] = '\0';
            printf("%s", buffer);
        }
    }

    free(line);

    close(pipes[0][1]);
    close(pipes[2][0]);

    int status;
    waitpid(child1_pid, &status, 0);
    waitpid(child2_pid, &status, 0);

    return 0;
}