#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

// (a * b) % mod без переполнения uint64_t
uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod);

// отправить ровно len байт из buf в сокет fd
// возвращает 0 при успехе, -1 при ошибке
int SendAll(int fd, const void *buf, size_t len);

// принять ровно len байт в buf из сокета fd
// возвращает 0 при успехе, -1 при ошибке или EOF
int RecvAll(int fd, void *buf, size_t len);

// разбить [begin, end] на n частей; вернуть границы i-й части (0-based)
// в *out_begin, *out_end. если кусок пуст — *out_begin = 1, *out_end = 0
void SplitRange(uint64_t begin, uint64_t end, uint64_t n, uint64_t i,
                uint64_t *out_begin, uint64_t *out_end);

#endif // COMMON_H