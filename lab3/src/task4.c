#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s seed arraysize\n", argv[0]);
    return 1;
  }

  int seed = atoi(argv[1]);
  if (seed <= 0) {
    printf("seed is a positive number\n");
    return 1;
  }

  int array_size = atoi(argv[2]);
  if (array_size <= 0) {
    printf("array_size is a positive number\n");
    return 1;
  }

  // argv[0] для новой программы — ее имя,
  // argv[1], argv[2] — seed и array_size, NULL — терминатор
  char *argv_seq[] = {"./sequential_min_max", argv[1], argv[2], NULL};

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    // ребенок: превращаемся в sequential_min_max
    execv("./sequential_min_max", argv_seq);
    // сюда попадаем только при ошибке execv
    perror("execv");
    _exit(1);
  }

  // родитель: ждем завершения ребёнка
  int status;
  waitpid(pid, &status, 0);

  if (WIFEXITED(status)) {
    printf("sequential_min_max exited with code %d\n", WEXITSTATUS(status));
  } else {
    printf("sequential_min_max terminated abnormally\n");
  }

  return 0;
}