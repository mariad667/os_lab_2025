// ./client --k ? --mod ? --servers servers.txt
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Server {
  char ip[255];
  int port;
};

struct ClientTask {
  struct Server server;
  uint64_t begin, end, mod;
  uint64_t result;
  bool ok;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a %= mod;
  while (b > 0) {
    if (b % 2 == 1) result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }
  return result % mod;
}

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  if (!str || !*str) return false;
  char *end = NULL;
  errno = 0;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }
  if (errno != 0 || end == str) return false;
  *val = (uint64_t)i;
  return true;
}

// Читает файл вида "ip:port" (по одному на строку). Возвращает количество серверов.
unsigned int ParseServersFile(const char *path, struct Server **out) {
  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Cannot open servers file '%s': %s\n", path, strerror(errno));
    return 0;
  }
  unsigned int cap = 8, n = 0;
  struct Server *arr = malloc(sizeof(struct Server) * cap);
  if (!arr) { fclose(f); return 0; }

  char line[512];
  while (fgets(line, sizeof(line), f)) {
    line[strcspn(line, "\r\n")] = '\0';
    if (line[0] == '\0' || line[0] == '#') continue;

    char *colon = strrchr(line, ':');
    if (!colon) {
      fprintf(stderr, "Bad line (no colon): %s\n", line);
      continue;
    }
    *colon = '\0';
    const char *ip = line;
    long port = strtol(colon + 1, NULL, 10);
    if (port <= 0 || port > 65535) {
      fprintf(stderr, "Bad port in line: %s\n", line);
      continue;
    }

    if (n == cap) {
      cap *= 2;
      struct Server *tmp = realloc(arr, sizeof(struct Server) * cap);
      if (!tmp) { free(arr); fclose(f); return 0; }
      arr = tmp;
    }
    strncpy(arr[n].ip, ip, sizeof(arr[n].ip) - 1);
    arr[n].ip[sizeof(arr[n].ip) - 1] = '\0';
    arr[n].port = (int)port;
    n++;
  }
  fclose(f);
  *out = arr;
  return n;
}

void *ServerWorker(void *arg) {
  struct ClientTask *t = (struct ClientTask *)arg;
  t->ok = false;
  t->result = 1;

  struct hostent *hostname = gethostbyname(t->server.ip);
  if (!hostname) {
    fprintf(stderr, "gethostbyname failed: %s\n", t->server.ip);
    return NULL;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((uint16_t)t->server.port);
memcpy(&addr.sin_addr, hostname->h_addr_list[0], (size_t)hostname->h_length);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) { perror("socket"); return NULL; }

  if (connect(sck, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    fprintf(stderr, "connect to %s:%d failed: %s\n",
            t->server.ip, t->server.port, strerror(errno));
    close(sck);
    return NULL;
  }

  char task[sizeof(uint64_t) * 3];
  memcpy(task, &t->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &t->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &t->mod, sizeof(uint64_t));

  size_t sent = 0;
  while (sent < sizeof(task)) {
    ssize_t n = send(sck, task + sent, sizeof(task) - sent, 0);
    if (n <= 0) { perror("send"); close(sck); return NULL; }
    sent += (size_t)n;
  }

  char response[sizeof(uint64_t)];
  size_t got = 0;
  while (got < sizeof(response)) {
    ssize_t n = recv(sck, response + got, sizeof(response) - got, 0);
    if (n <= 0) { perror("recv"); close(sck); return NULL; }
    got += (size_t)n;
  }

  memcpy(&t->result, response, sizeof(uint64_t));
  t->ok = true;

  close(sck);
  return NULL;
}

int main(int argc, char **argv) {
  uint64_t k = (uint64_t)-1;
  uint64_t mod = (uint64_t)-1;
  char servers_path[512] = {0};

  while (true) {
    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);
    if (c == -1) break;

    switch (c) {
    case 0:
      switch (option_index) {
      case 0:
        if (!ConvertStringToUI64(optarg, &k))
          fprintf(stderr, "Bad value for --k\n");
        break;
      case 1:
        if (!ConvertStringToUI64(optarg, &mod) || mod == 0)
          fprintf(stderr, "Bad value for --mod (must be > 0)\n");
        break;
      case 2:
        strncpy(servers_path, optarg, sizeof(servers_path) - 1);
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
      break;
    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == (uint64_t)-1 || mod == (uint64_t)-1 || mod == 0 || servers_path[0] == '\0') {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
    return 1;
  }

  struct Server *to = NULL;
  unsigned int servers_num = ParseServersFile(servers_path, &to);
  if (servers_num == 0) {
    fprintf(stderr, "No servers parsed from '%s'\n", servers_path);
    free(to);
    return 1;
  }

  struct ClientTask *tasks = malloc(sizeof(struct ClientTask) * servers_num);
  pthread_t *threads = malloc(sizeof(pthread_t) * servers_num);
  if (!tasks || !threads) {
    fprintf(stderr, "malloc failed\n");
    free(to); free(tasks); free(threads);
    return 1;
  }

  // делим диапазон [1, k] между серверами
  uint64_t total_range = k;  // от 1 до k включительно
  uint64_t chunk = total_range / servers_num;
  uint64_t rem = total_range % servers_num;
  uint64_t cur = 1;

  for (unsigned int i = 0; i < servers_num; i++) {
    uint64_t size = chunk + (i < rem ? 1 : 0);
    tasks[i].server = to[i];
    tasks[i].mod = mod;
    tasks[i].ok = false;
    tasks[i].result = 1;
    if (size == 0) {
      // пустой кусок: сервер вернет 1 (ничего не умножая)
      tasks[i].begin = 1;
      tasks[i].end = 0;
    } else {
      tasks[i].begin = cur;
      tasks[i].end = cur + size - 1;
      cur += size;
    }
  }

  // запускаем всех параллельно
  for (unsigned int i = 0; i < servers_num; i++) {
    if (pthread_create(&threads[i], NULL, ServerWorker, &tasks[i])) {
      fprintf(stderr, "pthread_create failed\n");
      free(to); free(tasks); free(threads);
      return 1;
    }
  }

  // собираем результаты
  uint64_t total = 1;
  bool all_ok = true;
  for (unsigned int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    if (!tasks[i].ok) {
      fprintf(stderr, "Server %s:%d failed\n",
              tasks[i].server.ip, tasks[i].server.port);
      all_ok = false;
      continue;
    }
    total = MultModulo(total, tasks[i].result, mod);
  }

  if (!all_ok) {
    fprintf(stderr, "Some servers failed; result may be wrong\n");
    free(to); free(tasks); free(threads);
    return 1;
  }

  printf("answer: %llu\n", (unsigned long long)total);
  fflush(stdout);

  free(to);
  free(tasks);
  free(threads);
  return 0;
}