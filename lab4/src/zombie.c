#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
  pid_t child_pid = fork();

  if (child_pid == -1) {
    perror("fork");
    return 1;
  }

  if (child_pid == 0) {
    printf("Ребенок (PID %d): зародился\n", getpid());
    _exit(0); // ребенок сразу завершается
  }

  // родитель
  printf("Родитель (PID %d): fфоркнул ребенка с PID %d\n", getpid(), child_pid);
  printf("Родитель: sleeping чтобы проверить статус ребенка...\n");

  sleep(15);   // время посмотреть на зомби в ps

  printf("Родитель: вызван wait() для уничтожения зомбака...\n");
  int status;
  waitpid(child_pid, &status, 0);

  if (WIFEXITED(status)) {
    printf("Родитель: избавился от ребенка, exit code = %d\n", WEXITSTATUS(status));
  }

  printf("Родитель: ребенка больше нет. sleeping чтобы убедиться...\n");
  sleep(10);

  printf("Родитель: завершает работу.\n");
  return 0;
}