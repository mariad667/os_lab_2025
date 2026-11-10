// task2
/* gcc -pthread -o task2 instasamka.c */
/* ./task2 -k 22 --pnum=10 --mod=113 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <string.h>

struct ThreadArgs {
  int begin;
  int end;
  long long mod;
  long long *total;
  pthread_mutex_t *mut;
};

void *ThreadFactorial(void *arg) {
  struct ThreadArgs *args = (struct ThreadArgs *)arg;

  long long local = 1;
  for (int i = args->begin + 1; i <= args->end; i++) {
    local = (local * i) % args->mod;
  }

  pthread_mutex_lock(args->mut);
  *args->total = (*args->total * local) % args->mod;
  pthread_mutex_unlock(args->mut);

  return NULL;
}

int main(int argc, char **argv) {
  int k = -1;
  int pnum = -1;
  long long mod = -1;

  while (1) {
    static struct option options[] = {
        {"pnum", required_argument, 0, 0},
        {"mod",  required_argument, 0, 0},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "k:", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("pnum must be a positive number\n");
              return 1;
            }
            break;
          case 1:
            mod = atoll(optarg);
            if (mod <= 0) {
              printf("mod must be a positive number\n");
              return 1;
            }
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'k':
        k = atoi(optarg);
        if (k <= 0) {
          printf("k must be a positive number\n");
          return 1;
        }
        break;
      case '?':
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (k == -1 || pnum == -1 || mod == -1) {
    printf("Usage: %s -k \"num\" --pnum=\"num\" --mod=\"num\"\n", argv[0]);
    return 1;
  }

  if (pnum > k) {
    printf("pnum must not be greater than k\n");
    return 1;
  }

  long long total = 1;
  pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

  pthread_t *threads = malloc(sizeof(pthread_t) * pnum);
  struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs) * pnum);
  if (!threads || !args) {
    perror("malloc");
    return 1;
  }

  for (int i = 0; i < pnum; i++) {
    args[i].begin = (i * k) / pnum;
    args[i].end   = ((i + 1) * k) / pnum;
    args[i].mod = mod;
    args[i].total = &total;
    args[i].mut = &mut;
  }

  for (int i = 0; i < pnum; i++) {
    int rc = pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i]);
    if (rc != 0) {
      printf("pthread_create failed: %d\n", rc);
      free(threads);
      free(args);
      return 1;
    }
  }

  for (int i = 0; i < pnum; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("Result: %lld\n", total);

  free(threads);
  free(args);
  return 0;
}