#include "common.h"

#include <errno.h>
#include <sys/socket.h>

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a %= mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }
  return result % mod;
}

int SendAll(int fd, const void *buf, size_t len) {
  const char *p = (const char *)buf;
  size_t sent = 0;
  while (sent < len) {
    ssize_t n = send(fd, p + sent, len - sent, 0);
    if (n < 0 && errno == EINTR) continue;
    if (n <= 0) return -1;
    sent += (size_t)n;
  }
  return 0;
}

int RecvAll(int fd, void *buf, size_t len) {
  char *p = (char *)buf;
  size_t got = 0;
  while (got < len) {
    ssize_t n = recv(fd, p + got, len - got, 0);
    if (n < 0 && errno == EINTR) continue;
    if (n == 0) return -1;   // соединение закрыто
    if (n < 0) return -1;
    got += (size_t)n;
  }
  return 0;
}

void SplitRange(uint64_t begin, uint64_t end, uint64_t n, uint64_t i,
                uint64_t *out_begin, uint64_t *out_end) {
  if (n == 0 || i >= n || end < begin) {
    *out_begin = 1;
    *out_end = 0;
    return;
  }
  uint64_t total = end - begin + 1;
  uint64_t chunk = total / n;
  uint64_t rem = total % n;

  uint64_t offset = i * chunk + (i < rem ? i : rem);
  uint64_t size = chunk + (i < rem ? 1 : 0);

  if (size == 0) {
    *out_begin = 1;
    *out_end = 0;
    return;
  }
  *out_begin = begin + offset;
  *out_end = begin + offset + size - 1;
}