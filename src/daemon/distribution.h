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

/*
 * NAME
 *  distribution_new_linear
 *
 * DESCRIPTION
 *  Creates new distribution structure and initialize buckets using linear
 *  function.
 *
 * ARGUMENTS
 *  num_buckets     Number of the buckets in the distribution.
 *  size            Size of every bucket.
 *
 * RETURN VALUE
 *  Returns pointer to the new distribution_t upon success or NULL if an error
 *  occurred. Error could occurred when num_buckets was zero or if OS couldn't
 *  allocate the memory - then errno will contain the error code.
 */
distribution_t *distribution_new_linear(size_t num_buckets, double size);
/*
 * NAME
 *  distribution_new_exponential
 *
 * DESCRIPTION
 *  Creates new distribution structure and initialize buckets using exponential
 *  function.
 *
 * ARGUMENTS
 *  num_buckets     Number of the buckets in the distribution.
 *  base            Base for calculating the size of bucket.
 *  factor          Multiplier for calculating the size of bucket.
 *
 * RETURN VALUE
 *  Returns pointer to the new distribution_t upon success or NULL if an error
 *  occurred. Error could occurred when num_buckets was zero or if OS couldn't
 *  allocate the memory - then errno will contain the error code.
 */
distribution_t *distribution_new_exponential(size_t num_buckets, double base,
                                             double factor);
/*
 * NAME
 *  distribution_new_custom
 *
 * DESCRIPTION
 *  Creates new distribution structure and initialize buckets using custom
 * buckets boundaries given by the user
 *
 * ARGUMENTS
 *  num_buckets                  Number of the buckets in the distribution.
 *  custom_buckets_boundaries    Boundaries of buckets in ascending order.
 *
 * RETURN VALUE
 *  Returns pointer to the new distribution_t upon success or NULL if an error
 *  occurred. Error could occurred when num_buckets was zero or if OS couldn't
 *  allocate the memory - then errno will contain the error code. There is also
 * one case when the function can return with some error - if the custom buckets
 * boundaries aren't in the ascending order or some boundaries are less than
 * zero or are equal to INFINITY, then the function will return null and will
 * set errno to EINVAL.
 */
distribution_t *distribution_new_custom(size_t num_boundaries,
                                        double *custom_buckets_boundaries);
/*
 * NAME
 *  distribution_update
 *
 * DESCRIPTION
 *  Updates the given distribution with provided gauge. Increase the counter of
 * the proper bucket for given gauge.
 *
 * ARGUMENTS
 *  d               Pointer to the distribution.
 *  gauge           Value of gauge.
 *
 * RETURN VALUE
 *  Returns zero upon success or -1 if an error occurred. Error could occurred
 * when the user will give the wrong argument, i.e. d will be null, then the
 * function will return -1 and the errno will be set to EINVAL.
 */
int distribution_update(distribution_t *d, double gauge);
/*
 * NAME
 *  distribution_percentile
 *
 * DESCRIPTION
 *  Calculates the percentile for the given distribution and provided percent.
 *
 * ARGUMENTS
 *  d               Pointer to the distribution.
 *  percent         Percentile that the function should return.
 *
 * RETURN VALUE
 *  Returns the maximum boundary of the bucket in which should be the percent
 * given as an argument upon success or NAN if an error occurred. Error could
 * occurred when the user will give the wrong argument, i.e. d will be null or
 * percent will be greater than 100.0, then the function will return NaN and the
 * errno will be set to EINVAL.
 */
double distribution_percentile(distribution_t *d, double percent);
/*
 * NAME
 *  distribution_average
 *
 * DESCRIPTION
 *  Calculates average of gauges in the given distribution.
 *
 * ARGUMENTS
 *  d               Pointer to the distribution.
 *
 * RETURN VALUE
 *  Returns the average of all gauges in the given distribution upon success or
 * NAN if an error occurred. Error could occurred when the user will pass the
 * wrong argument to the function, i.e. d will be null, then the function will
 * return NaN and the errno will be set to EINVAL or there were zero updates in
 * this distribution.
 */
double distribution_average(distribution_t *d);
/*
 * NAME
 *  distribution_clone
 *
 * DESCRIPTION
 *  Clones the distribution given as an argument.
 *
 * ARGUMENTS
 *  d               Pointer to the distribution.
 *
 * RETURN VALUE
 *  Returns the pointer to the cloned distribution upon success or
 * NULL if an error occurred. Error could occurred when the user will pass the
 * wrong argument to this function, i.e. d will be null, then the function will
 * return null and the errno will be set to EINVAL there is also the possibility
 * that the user will pass the proper argument but the function will fail
 * anyway, i.e. calloc will return null, then the function will return null and
 * the errno will be set by calloc.
 */
distribution_t *distribution_clone(distribution_t *d);
/*
 * NAME
 *  distribution_destroy
 *
 * DESCRIPTION
 *  Cleans up and frees all the memory
 *
 * ARGUMENTS
 *  d               Pointer to the distribution.
 */
void distribution_destroy(distribution_t *d);
/*
 * NAME
 *  distribution_check_equal
 *
 * DESCRIPTION
 *  Checks if the distributions given as arguments are equal.
 *
 * ARGUMENTS
 *  d1               Pointer to the first distribution.
 *  d2               Pointer to the second distribution.
 *
 * RETURN VALUE
 *  Returns the true if the distributions given as arguments are equal
 *  or false if they aren't.
 */
bool distribution_check_equal(distribution_t *d1, distribution_t *d2);
/*
 * NAME
 *  distribution_get_buckets_boundaries
 *
 * DESCRIPTION
 *  Gets buckets' boundaries.
 *
 * ARGUMENTS
 *  d               Pointer to the distribution.
 *
 * RETURN VALUE
 *  Returns the pointer to the array with boundaries of buckets upon success or
 * NULL if an error occurred. Error could occurred when the user gave the NULL
 * as an argument, then function will return NULL and set errno to EINVAL or if
 * calling calloc inside function failed. Then errno will be set by calloc and
 * NULL be returned.
 */
double *distribution_get_buckets_boundaries(distribution_t *d);
/*
 * NAME
 *  distribution_get_buckets_counters
 *
 * DESCRIPTION
 *  Gets buckets' counters.
 *
 * ARGUMENTS
 *  d               Pointer to the distribution.
 *
 * RETURN VALUE
 *  Returns the pointer to the array with counters of buckets upon success or
 * NULL if an error occurred. Error could occurred when the user gave the NULL
 * as an argument, then function will return NULL and set errno to EINVAL or if
 * calling calloc inside function failed. Then errno will be set by calloc and
 * NULL be returned.
 */
uint64_t *distribution_get_buckets_counters(distribution_t *d);
/*
 * NAME
 *  distribution_get_num_buckets
 *
 * DESCRIPTION
 *  Gets number of buckets in the distribution.
 *
 * ARGUMENTS
 *  d               Pointer to the distribution.
 *
 * RETURN VALUE
 *  Returns the number of buckets in the distribution upon success or 0
 * if an error occurred. Error could occurred when the user gave the NULL as an
 * argument, then function will return 0.
 */
size_t distribution_get_num_buckets(distribution_t *d);
/*
 * NAME
 *  distribution_get_sum_gauges
 *
 * DESCRIPTION
 *  Gets sum of gauges in the distribution.
 *
 * ARGUMENTS
 *  d               Pointer to the distribution.
 *
 * RETURN VALUE
 *  Returns the sum of gauges in the distribution upon success or 0
 * if an error occurred. Error could occurred when the user gave the NULL as an
 * argument, then function will return 0.
 */
double distribution_get_sum_gauges(distribution_t *d);

#endif // DISTRIBUTION_H
