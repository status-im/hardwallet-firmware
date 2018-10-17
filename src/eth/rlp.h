/**
 * This file is part of the Status project, https://status.im/
 *
 * Copyright (c) 2018 Status Research & Development GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef RLP_H_
#define RLP_H_

#include <stdint.h>

/**
 *  Takes an RLP item, returns its length and stores the pointer to the value and to the next item in the given parameters.
 *  The barrier parameter is the address immediately after the transaction buffer. If parsing produces any address (value or next)
 *  which is larger than the barrier, parsing will fail. This is a protection against malformed data.
 *
 *  On failure this function returns -1 and. Value and next pointer will point to NULL. If the address of next is exactly the barrier, then
 *  the function will not fail but the next pointer will point to NULL, indicating that this was the last field.
 *
 **/
int rlp_parse(const uint8_t* item, uint8_t** value, uint8_t** next, const uint8_t* barrier);

/**
 *  Takes an RLP item and reads its content as a big endian uint32. Returns -1 on failure, 0 otherwise.
 */
int rlp_read_uint32(const uint8_t* item, uint32_t* value, uint8_t** next, const uint8_t* barrier);

/**
 * Returns the number of bytes needed to encode the given len.
 */
int rlp_len_of_len(int len);

/**
 * Writes the given length to the buffer. The is_list parameter encodes if the element is a list.
 */
void rlp_write_len(uint8_t* buf, int len, int is_list);

#endif /* RLP_H_ */
