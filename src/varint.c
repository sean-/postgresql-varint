/* PostgreSQL Varint Extension
 * Copyright 2012 Sean Chittenden.
 *
 * The encoding scheme and portions of this software are derived from
 * Protobuf.
 *
 * Protobuf copyright:
 * Copyright 2008 Kenton Vara, Google Inc.
 *
 * Encoding method used is based on original Protocol Buffers design by
 * Sanjay Ghemawat, Jeff Dean, and others.
 *
 * See ../COPYING for additional copyright and license information.
 */

#include "varint.h"

void
varint_to_uint64(const char* buf, const size_t buflen, uint64_t* out, size_t* consumed) {
  const char* ptr = buf;
  uint64_t b;
  /* According to Google, splitting into 32-bit pieces gives better
   * performace on 32-bit processors. */
  uint32_t part0 = 0, part1 = 0, part2 = 0;

  if (buflen <  1) goto err; b = *(ptr++); part0  = (b & 0x7F);       if (!(b & 0x80)) goto done;
  if (buflen <  2) goto err; b = *(ptr++); part0 |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
  if (buflen <  3) goto err; b = *(ptr++); part0 |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
  if (buflen <  4) goto err; b = *(ptr++); part0 |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
  if (buflen <  5) goto err; b = *(ptr++); part1  = (b & 0x7F);       if (!(b & 0x80)) goto done;
  if (buflen <  6) goto err; b = *(ptr++); part1 |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
  if (buflen <  7) goto err; b = *(ptr++); part1 |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
  if (buflen <  8) goto err; b = *(ptr++); part1 |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
  if (buflen <  9) goto err; b = *(ptr++); part2  = (b & 0x7F);       if (!(b & 0x80)) goto done;
  if (buflen < 10) goto err; b = *(ptr++); part2 |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;

  /* Buffer over run: the max size of a varint encoded uint64_t is 10 bytes. */
done:
  *consumed = ptr - buf;
  *out = (part0 | ((uint64_t)(part1) << 28) | ((uint64_t)(part2) << 56));
  return;
err:
  *consumed = 0;
  return;
}


void
uint64_to_varint(const uint64_t in, const size_t buflen, char* buf, size_t* output_len) {
  uint32_t part0 = in;
  uint32_t part1 = (in >> 28);
  uint32_t part2 = (in >> 56);

  int sz;
  if (part2 == 0) {
    if (part1 == 0) {
      if (part0 < (1 << 14)) {
        if (part0 < (1 << 7)) {
          sz = 1; if (buflen >= 1) goto size1; else goto error;
        } else {
          sz = 2; if (buflen >= 2) goto size2; else goto error;
        }
      } else {
        if (part0 < (1 << 21)) {
          sz = 3; if (buflen >= 3) goto size3; else goto error;
        } else {
          sz = 4; if (buflen >= 4) goto size4; else goto error;
        }
      }
    } else {
      if (part1 < (1 << 14)) {
        if (part1 < (1 << 7)) {
          sz = 5; if (buflen >= 5) goto size5; else goto error;
        } else {
          sz = 6; if (buflen >= 6) goto size6; else goto error;
        }
      } else {
        if (part1 < (1 << 21)) {
          sz = 7; if (buflen >= 7) goto size7; else goto error;
        } else {
          sz = 8; if (buflen >= 8) goto size8; else goto error;
        }
      }
    }
  } else {
    if (part2 < (1 << 7)) {
      sz = 9; if (buflen >= 9) goto size9; else goto error;
    } else {
      sz = 10; if (buflen >= 10) goto size10; else goto error;
    }
  }

  size10: buf[9] = (unsigned char)((part2 >>  7) | 0x80);
  size9 : buf[8] = (unsigned char)((part2      ) | 0x80);
  size8 : buf[7] = (unsigned char)((part1 >> 21) | 0x80);
  size7 : buf[6] = (unsigned char)((part1 >> 14) | 0x80);
  size6 : buf[5] = (unsigned char)((part1 >>  7) | 0x80);
  size5 : buf[4] = (unsigned char)((part1      ) | 0x80);
  size4 : buf[3] = (unsigned char)((part0 >> 21) | 0x80);
  size3 : buf[2] = (unsigned char)((part0 >> 14) | 0x80);
  size2 : buf[1] = (unsigned char)((part0 >>  7) | 0x80);
  size1 : buf[0] = (unsigned char)((part0      ) | 0x80);
  buf[sz - 1] &= 0x7F;
  *output_len = sz;
  return;
error:
  output_len = 0;
  return;
}
