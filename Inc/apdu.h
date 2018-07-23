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

#ifndef APDU_H_
#define APDU_H_

#include <stdint.h>

typedef int32_t apdu_status_t;

#define APDU_STATUS_INVALID_SEGMENT 0xC0000000
#define APDU_STATUS_PARTIAL 0x80000000
#define APDU_STATUS_COMPLETE 0x40000000

apdu_status_t apdu_receive_segment(uint8_t* in, int len);
int apdu_process();
apdu_status_t apdu_send_segment(uint8_t* out, int len);

#endif /* APDU_H_ */
