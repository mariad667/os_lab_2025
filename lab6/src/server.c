// ./server --port ? --tnum ?
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
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"

struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }
  return result % mod;
}

uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;
  for (uint64_t i = args->begin; i <= args->end; i++) {
    ans = MultModulo(ans, i, args->mod);
    if (i == UINT64_MAX) break;  // защита от переполнения счетчика
  }
  return ans;
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  uint64_t *res = malloc(sizeof(uint64_t));
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
    case '?':
      printf("Unknown argument\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
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
      const unsigned int buffer_size = sizeof(uint64_t) * 3;
      char from_client[sizeof(uint64_t) * 3];

      // корректное дочитывание потому что tcp может вернуть меньше чем просили
      size_t got = 0;
      while (got < buffer_size) {
        ssize_t r = recv(client_fd, from_client + got, buffer_size - got, 0);
        if (r == 0) { got = 0; break; }                // соединение закрыто
        if (r < 0) { perror("recv"); got = 0; break; }
        got += (size_t)r;
      }
      if (got < buffer_size) break;

      uint64_t begin = 0, end = 0, mod = 1;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end,   from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod,   from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

      fprintf(stdout, "Receive: %llu %llu %llu\n",
              (unsigned long long)begin,
              (unsigned long long)end,
              (unsigned long long)mod);
      fflush(stdout);

      // разбиваем [begin, end] на tnum частей
      struct FactorialArgs args[tnum];
      pthread_t threads[tnum];

      uint64_t total_range = (end >= begin) ? (end - begin + 1) : 0;
      uint64_t chunk = (tnum > 0) ? (total_range / (uint64_t)tnum) : 0;
      uint64_t rem   = (tnum > 0) ? (total_range % (uint64_t)tnum) : 0;
      uint64_t cur   = begin;

      for (int i = 0; i < tnum; i++) {
        uint64_t size = chunk + ((uint64_t)i < rem ? 1 : 0);
        if (size == 0) {
          // пустой кусок: begin > end, factorial вернет 1
          args[i].begin = 1;
          args[i].end   = 0;
        } else {
          args[i].begin = cur;
          args[i].end   = cur + size - 1;
          cur += size;
        }
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
        uint64_t part = *(uint64_t *)ret;
        free(ret);
        total = MultModulo(total, part, mod);
      }

      printf("Total: %llu\n", (unsigned long long)total);
      fflush(stdout);

      char buf[sizeof(uint64_t)];
      memcpy(buf, &total, sizeof(uint64_t));
      size_t sent = 0;
      while (sent < sizeof(buf)) {
        ssize_t s = send(client_fd, buf + sent, sizeof(buf) - sent, 0);
        if (s <= 0) { perror("send"); break; }
        sent += (size_t)s;
      }
    }

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
  }

  return 0;
}