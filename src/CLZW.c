#include <CLZW.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* -------- aux stuff ---------- */
void* lzw_mem_alloc(size_t item_size, size_t n_item) {
  size_t *x = calloc(1, sizeof(size_t)*2 + n_item * item_size);
  x[0] = item_size;
  x[1] = n_item;
  return x + 2;
}

void* lzw_mem_extend(void *m, size_t new_n) {
  size_t *x = (size_t*)m - 2;
  x = realloc(x, sizeof(size_t) * 2 + *x * new_n);
  if (new_n > x[1])
    memset((char*)(x + 2) + x[0] * x[1], 0, x[0] * (new_n - x[1]));
  x[1] = new_n;
  return x + 2;
}

void mem_clear(void *m) {
  size_t *x = (size_t*)m - 2;
  memset(m, 0, x[0] * x[1]);
}

#define _setsize(m, n) m = lzw_mem_extend(m, n)
#define _extend(m) m = lzw_mem_extend(m, lzw_len(m) * 2)

/* ----------- LZW stuff -------------- */
typedef uint16_t ushort;

#define M_CLR 256 /* clear table marker */
#define M_EOD 257 /* end-of-data marker */
#define M_NEW 258 /* new code index */

/* encode and decode dictionary structures.
   for encoding, entry at code index is a list of indices that follow current one,
   i.e. if code 97 is 'a', code 387 is 'ab', and code 1022 is 'abc',
   then dict[97].next['b'] = 387, dict[387].next['c'] = 1022, etc. */
typedef struct {
  ushort next[256];
} lzw_enc_t;

/* for decoding, dictionary contains index of whatever prefix index plus trailing
   byte.  i.e. like previous example,
    dict[1022] = { c: 'c', prev: 387 },
    dict[387]  = { c: 'b', prev: 97 },
    dict[97]   = { c: 'a', prev: 0 }
   the "back" element is used for temporarily chaining indices when resolving
   a code to bytes
 */
typedef struct {
  ushort prev, back;
  lzw_byte c;
} lzw_dec_t;

lzw_byte* lzw_encode(lzw_byte *in, int max_bits)
{
  int len = lzw_len(in), bits = 9, next_shift = 512;
  ushort code, c, nc, next_code = M_NEW;
  lzw_enc_t *d = lzw_new(lzw_enc_t, 512);

  if (max_bits > 15) max_bits = 15;
  if (max_bits < 9 ) max_bits = 12;

  lzw_byte *out = lzw_new(ushort, 4);
  int out_len = 0, o_bits = 0;
  uint32_t tmp = 0;

  inline void write_bits(ushort x) {
    tmp = (tmp << bits) | x;
    o_bits += bits;
    if (lzw_len(out) <= out_len) _extend(out);
    while (o_bits >= 8) {
      o_bits -= 8;
      out[out_len++] = tmp >> o_bits;
      tmp &= (1 << o_bits) - 1;
    }
  }

  //write_bits(M_CLR);
  for (code = *(in++); --len; ) {
    c = *(in++);
    if ((nc = d[code].next[c]))
      code = nc;
    else {
      write_bits(code);
      nc = d[code].next[c] = next_code++;
      code = c;
    }

    /* next new code would be too long for current table */
    if (next_code == next_shift) {
      /* either reset table back to 9 bits */
      if (++bits > max_bits) {
        /* table clear marker must occur before bit reset */
        write_bits(M_CLR);

        bits = 9;
        next_shift = 512;
        next_code = M_NEW;
        mem_clear(d);
      } else  /* or extend table */
        _setsize(d, next_shift *= 2);
    }
  }

  write_bits(code);
  write_bits(M_EOD);
  if (tmp) write_bits(tmp);

  lzw_del(d);

  _setsize(out, out_len);
  return out;
}

lzw_byte* lzw_decode(lzw_byte *in)
{
  lzw_byte *out = lzw_new(lzw_byte, 4);
  int out_len = 0;

  inline void write_out(lzw_byte c)
  {
    while (out_len >= lzw_len(out)) _extend(out);
    out[out_len++] = c;
  }

  lzw_dec_t *d = lzw_new(lzw_dec_t, 512);
  int len, j, next_shift = 512, bits = 9, n_bits = 0;
  ushort code, c, t, next_code = M_NEW;

  uint32_t tmp = 0;
  inline void get_code() {
    while(n_bits < bits) {
      if (len > 0) {
        len --;
        tmp = (tmp << 8) | *(in++);
        n_bits += 8;
      } else {
        tmp = tmp << (bits - n_bits);
        n_bits = bits;
      }
    }
    n_bits -= bits;
    code = tmp >> n_bits;
    tmp &= (1 << n_bits) - 1;
  }

  inline void clear_table() {
    mem_clear(d);
    for (j = 0; j < 256; j++) d[j].c = j;
    next_code = M_NEW;
    next_shift = 512;
    bits = 9;
  };

  clear_table(); /* in case encoded bits didn't start with M_CLR */
  for (len = lzw_len(in); len;) {
    get_code();
    if (code == M_EOD) break;
    if (code == M_CLR) {
      clear_table();
      continue;
    }

    if (code >= next_code) {
      fprintf(stderr, "Bad sequence\n");
      lzw_del(out);
      goto bail;
    }

    d[next_code].prev = c = code;
    while (c > 255) {
      t = d[c].prev; d[t].back = c; c = t;
    }

    d[next_code - 1].c = c;

    while (d[c].back) {
      write_out(d[c].c);
      t = d[c].back; d[c].back = 0; c = t;
    }
    write_out(d[c].c);

    if (++next_code >= next_shift) {
      if (++bits > 16) {
        /* if input was correct, we'd have hit M_CLR before this */
        fprintf(stderr, "Too many bits\n");
        lzw_del(out);
        goto bail;
      }
      _setsize(d, next_shift *= 2);
    }
  }

  /* might be ok, so just whine, don't be drastic */
  if (code != M_EOD)
    fputs("Bits did not end in EOD\n", stderr);

  _setsize(out, out_len);

 bail:
  lzw_del(d);

  return out;
}
