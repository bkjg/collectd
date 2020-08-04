/**
 * collectd - src/daemon/distribution.h
 * Copyright (C) 2020       Google LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *   Barbara bkjg Kaczorowska <bkjg at google.com>
 */

#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include "collectd.h"

struct distribution_s;
typedef struct distribution_s distribution_t;

/* function that create new distribution structure and initialize buckets using
 * linear function
 * it will return null if any error occurred, for example - num_buckets is zero
 * or OS couldn't allocate the memory - then errno will contain the error code
 */
distribution_t *distribution_new_linear(size_t num_buckets, double size);
/* function that create new distribution structure and initialize buckets using
 * exponential function
 * it will return null if any error occurred, for example - num_buckets is zero
 * or OS couldn't allocate the memory - then errno will contain the error code
 */
distribution_t *distribution_new_exponential(size_t num_buckets, double base,
                                             double factor);
/* function that create new distribution structure and initialize buckets using
 * custom buckets sizes given by the user
 * It will return null if any error occurred, for example - num_boundaries is
 * less than zero or OS couldn't allocate the memory - then errno will contain
 * the error code There is also one case when the function can return with some
 * error - if the custom buckets boundaries aren't in the ascending order or
 * some boundaries are less than zero, then the function will return null and
 * will set errno to EINVAL
 */
distribution_t *distribution_new_custom(size_t num_boundaries,
                                        double *custom_buckets_boundaries);

/* function for updating the buckets
 * if the user will give the wrong argument, i.e. d will be null, then the
 * function will return and the errno will be set to EINVAL*/
int distribution_update(distribution_t *d, double gauge);

/* function  for getting the percentile
 * if the user will give the wrong argument, i.e. d will be null or percent
 * will be greater than 100.0, then the function will return NaN and the errno
 * will be set to EINVAL*/
double distribution_percentile(distribution_t *d, double percent);

/* function that calculates average of gauges
 * if the user will pass the wrong argument to the function, i.e. d will be
 * null, then the function will return NaN and the errno will be set to EINVAL
 */
double distribution_average(distribution_t *d);

/* function that do the clone of distribution structure
 * if the user will pass the wrong argument to this function, i.e. d will be
 * null, then the function will return null and the errno will be set to EINVAL
 * there is also the possibility that the user will pass the proper argument but
 * the function will fail anyway, i.e. calloc will return null, then the
 * function will return null and the errno will be set by calloc
 */
distribution_t *distribution_clone(distribution_t *d);

/* function that do clean up and free all the memory
 * if the user will pass the null as an argument to this function, then the
 * function
 * will return without setting any errno like the OS do it when have to free a
 * null pointer */
void distribution_destroy(distribution_t *d);

/* TODO(bkjg): add descriptions of these functions and improve previous
 * descriptions */
bool distribution_check_equal(distribution_t *d1, distribution_t *d2);

double *distribution_get_buckets_boundaries(distribution_t *d);

uint64_t *distribution_get_buckets_counters(distribution_t *d);

size_t distribution_get_num_buckets(distribution_t *d);

double distribution_get_sum_gauges(distribution_t *d);

#endif // DISTRIBUTION_H
