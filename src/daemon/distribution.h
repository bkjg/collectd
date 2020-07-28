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
distribution_t *distribution_new_exponential(size_t num_buckets,
                                             double initial_size,
                                             double factor);
/* function that create new distribution structure and initialize buckets using
 * custom buckets sizes given by the user
 * it will return null if any error occurred, for example - num_buckets is zero
 * or OS couldn't allocate the memory - then errno will contain the error code
 */
distribution_t *distribution_new_custom(size_t num_buckets,
                                        double *custom_buckets_boundaries);

/* function for updating the buckets
 * if the user will give the wrong argument, i.e. d will be null, then the
 * function will return and the errno will be set to EINVAL*/
void distribution_update(distribution_t *d, double gauge);

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
 * function will return null and the errno will be set by calloc system call
 */
distribution_t *distribution_clone(distribution_t *d);

/* function that do clean up and free all the memory
 * if the user will pass the null as an argument to this function, then the
 * function
 * will return without setting any errno like the OS do it when have to free a
 * null pointer */
void distribution_destroy(distribution_t *d);

#endif // DISTRIBUTION_H
