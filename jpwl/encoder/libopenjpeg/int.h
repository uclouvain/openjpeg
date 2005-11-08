/*
 * Copyright (c) 2001-2002, David Janssens
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __INT_H
#define __INT_H

/*
 * Get the minimum of two integers.
 *
 * returns a if a < b else b
 */
int int_min(int a, int b);

/*
 * Get the maximum of two integers.
 *
 * returns a if a > b else b
 */
int int_max(int a, int b);

/*
 * Clamp an integer inside an interval.
 *
 * return a if (min < a < max)
 * return max if (a > max)
 * return min if (a < min) 
 */
int int_clamp(int a, int min, int max);

/*
 * Get absolute value of integer.
 */
int int_abs(int a);

/*
 * Divide an integer and round upwards.
 *
 * a divided by b
 */
int int_ceildiv(int a, int b);

/*
 * Divide an integer by a power of 2 and round upwards.
 *
 * a divided by 2^b
 */
int int_ceildivpow2(int a, int b);

/*
 * Divide an integer by a power of 2 and round downwards.
 *
 * a divided by 2^b
 */
int int_floordivpow2(int a, int b);

/*
 * Get logarithm of an integer and round downwards.
 *
 * log2(a)
 */
int int_floorlog2(int a);

#endif
