#ifndef CLZW_H
#define CLZW_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

typedef uint8_t lzw_byte;

#ifdef __cplusplus
  #define lzw_new(type, n) lzw_mem_alloc(sizeof(type), n)
  #define lzw_del(m) { free(reinterpret_cast<size_t *>(m) - 2); m = nullptr; }
  #define lzw_len(m) *(reinterpret_cast<size_t *>(m) - 1)
#else
  #define lzw_new(type, n) lzw_mem_alloc(sizeof(type), n)
  #define lzw_del(m) { free((size_t*)(m) - 2); m = 0; }
  #define lzw_len(m) *((size_t*)m - 1)
#endif

lzw_byte *lzw_encode(lzw_byte *in, int max_bits);
lzw_byte *lzw_decode(lzw_byte *in);

void* lzw_mem_alloc(size_t item_size, size_t n_item);
void* lzw_mem_extend(void *m, size_t new_n);

#endif
