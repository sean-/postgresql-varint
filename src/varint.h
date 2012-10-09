#ifndef POSTGRESQL_VARINT_H
#define POSTGRESQL_VARINT_H

/**
 * @file varint.h
 * @author Sean Chittenden <sean@chittenden.org>
 * @date 2012-10-08
 * @brief varint encoding and decoding routines provide a reliable and dynamic method of serializing unsigned 64bit integers.
 *
 * Copyright 2012 Sean Chittenden.
 * See ../COPYING for additional copyright and license information.
 *
 * Storing data as a uint64 provides a net storage savings for most data. For
 * example, unlike normal 64 bit binary encoding, values between 0 and 127
 * consume only one byte of storage on disk.
 *
 * For values between 0 and 562949953421311 (0x1ffffffffffff, 2 ** 49 - 1),
 * uint64 will save anywhere between 1-7 bytes of storage.
 *
 * For values between 562949953421312 (0x2000000000000, 2 ** 49) and
 * 72057594037927935 (0xffffffffffffff, 2 ** 56 - 1) the space savings is a wash.
 *
 * For values greater than 72057594037927936 (0x100000000000000, 2 ** 56) and
 * less than 2 ** 64 -1, the space savings costs an extra 1-2 bytes.
 *
 *  1 byte:                    0 -                  127
 *  2 bytes:                 128 -                16383
 *  3 bytes:               16384 -              2097151
 *  4 bytes:             2097152 -            268435455
 *  5 bytes:           268435456 -          34359738367
 *  6 bytes:         34359738368 -        4398046511103
 *  7 bytes:       4398046511104 -      562949953421311
 *  8 bytes:     562949953421312 -    72057594037927935
 *  9 bytes:   72057594037927936 -  9223372036854775807
 * 10 bytes: 9223372036854775808 - 18446744073709551615
 *
 * For most people, there is probably a large net savings for on-disk storage
 * as a result of using varint encoding.
 */

#include <stdint.h>
#include <unistd.h>

/*!
 * Encode a uint64_t on a preallocated buffer
 * @param[in]      in The uint64_t input
 * @param[in]   bufsz The size of the buffer being used to serialize \$in
 * @param[out]    buf The output buffer for the serialized uint64_t \var value
 * @param[out] buflen The amount of buffer consumed
 */
void uint64_to_varint(const uint64_t in, const size_t bufsz, char* buf, size_t* buflen);

/*!
 * Converts the varint encoded unsigned integer back to a uint64_t
 * @param[in]      in The uint64_t input
 * @param[in]   bufsz The size of the buffer being used to serialize \$in
 * @param[out]    buf The output buffer for the serialized uint64_t \var value
 * @param[out] buflen The amount of buffer consumed
 */
void varint_to_uint64(const char* buf, const size_t bufsz, uint64_t* out, size_t* consumed);

#endif /* POSTGRESQL_VARINT_H */
