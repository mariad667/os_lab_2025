#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "common.h"

struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;
  for (uint64_t i = args->begin; i <= args->end; i++) {
    ans = MultModulo(ans, i, args->mod);
    if (i == UINT64_MAX) break;
  }
  return ans;
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  uint64_t *res = malloc(sizeof(uint64_t));
  if (!res) return NULL;
  *res = Factorial(fargs);
  return res;
}

int main(int argc, char **argv) {
  int tnum = -1;
  int port = -1;

  while (true) {
    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);
    if (c == -1) break;

    switch (c) {
    case 0:
      switch (option_index) {
      case 0: port = atoi(optarg); break;
      case 1: tnum = atoi(optarg); break;
      default: printf("Index %d is out of options\n", option_index);
      }
      break;
    case '?': printf("Unknown argument\n"); break;
    default: fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (port == -1 || tnum == -1 || tnum <= 0) {
    fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) { perror("socket"); return 1; }

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("bind");
    return 1;
  }
  if (listen(server_fd, 128) < 0) {
    perror("listen");
    return 1;
  }

  printf("Server listening at %d\n", port);
  fflush(stdout);

  while (true) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);
    if (client_fd < 0) { perror("accept"); continue; }

    while (true) {
      char from_client[sizeof(uint64_t) * 3];
      if (RecvAll(client_fd, from_client, sizeof(from_client)) < 0)
        break;

      uint64_t begin = 0, end = 0, mod = 1;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end,   from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod,   from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

      fprintf(stdout, "Receive: %llu %llu %llu\n",
              (unsigned long long)begin,
              (unsigned long long)end,
              (unsigned long long)mod);
      fflush(stdout);

      struct FactorialArgs args[tnum];
      pthread_t threads[tnum];

      for (int i = 0; i < tnum; i++) {
        SplitRange(begin, end, (uint64_t)tnum, (uint64_t)i,
                   &args[i].begin, &args[i].end);
        args[i].mod = mod;
        if (pthread_create(&threads[i], NULL, ThreadFactorial, &args[i])) {
          fprintf(stderr, "pthread_create failed\n");
          close(client_fd);
          return 1;
        }
      }

      uint64_t total = 1;
      for (int i = 0; i < tnum; i++) {
        void *ret = NULL;
        pthread_join(threads[i], &ret);
        if (ret) {
          uint64_t part = *(uint64_t *)ret;
          free(ret);
          total = MultModulo(total, part, mod);
        }
      }

      printf("Total: %llu\n", (unsigned long long)total);
      fflush(stdout);

      char buf[sizeof(uint64_t)];
      memcpy(buf, &total, sizeof(uint64_t));
      if (SendAll(client_fd, buf, sizeof(buf)) < 0) {
        perror("send");
        break;
      }
    }

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
  }

  return 0;
}