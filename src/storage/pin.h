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

#ifndef PIN_H_
#define PIN_H_

#include <stdint.h>

#define PIN_MIN_LEN 4
#define PIN_MAX_LEN 15

/**
 * Sets the PIN. Returns 0 on success, -1 on failure. Only works if no PIN is currently set.
 */
int pin_set(uint8_t* pin);

/**
 * Returns 1 if a PIN is set, 0 otherwise.
 */
int pin_is_set();

/**
 * Changes the PIN. Returns 1 on success and -1 on failure or if the current PIN has not been verified in this session.
 */
int pin_change(uint8_t* new_pin);

/**
 * Verifies the PIN. Returns 0 if the old PIN could not be verified, 1 on success and -1 on failure. The pin_is_verified()
 * function will always return 0 after a failed verification attempt, even if the PIN was successfully verified before.
 */
int pin_verify(uint8_t* pin);

/**
 * Returns 1 if the PIN has been verified in the current session. Returns 0 otherwise
 */
int pin_is_verified();

/**
 * Resets the PIN verification status.
 */
void pin_unverify();

/**
 * Returns the number of remaining retries. Returns -1 in case of internal error.
 */
int pin_remaining_tries();

#endif /* PIN_H_ */
