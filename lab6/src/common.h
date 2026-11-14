#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod);

int SendAll(int fd, const void *buf, size_t len);
int RecvAll(int fd, void *buf, size_t len);

void SplitRange(uint64_t begin, uint64_t end, uint64_t n, uint64_t i,
                uint64_t *out_begin, uint64_t *out_end);

#endif