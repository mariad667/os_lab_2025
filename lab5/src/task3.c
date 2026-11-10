// ps aux | grep deadlock
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mut1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut2 = PTHREAD_MUTEX_INITIALIZER;

void *thread1_func(void *arg) {
  (void)arg;
  printf("Thread 1: locking mut1\n");
  pthread_mutex_lock(&mut1);
  printf("Thread 1: locked mut1, sleeping 1s\n");
  sleep(1);   // даем thread2 захватить mut2

  printf("Thread 1: trying to lock mut2\n");
  pthread_mutex_lock(&mut2);   // а тут блокируется навсегда
  printf("Thread 1: locked mut2 (never printed)\n");

  pthread_mutex_unlock(&mut2);
  pthread_mutex_unlock(&mut1);
  return NULL;
}

void *thread2_func(void *arg) {
  (void)arg;
  printf("Thread 2: locking mut2\n");
  pthread_mutex_lock(&mut2);
  printf("Thread 2: locked mut2, sleeping 1s\n");
  sleep(1);

  printf("Thread 2: trying to lock mut1\n");
  pthread_mutex_lock(&mut1);    // блокируется навсегда
  printf("Thread 2: locked mut1 (never printed)\n");

  pthread_mutex_unlock(&mut1);
  pthread_mutex_unlock(&mut2);
  return NULL;
}

int main(void) {
  pthread_t t1, t2;

  pthread_create(&t1, NULL, thread1_func, NULL);
  pthread_create(&t2, NULL, thread2_func, NULL);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  printf("Done (never printed)\n");
  return 0;
}