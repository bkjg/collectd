#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

struct distribution_s;
typedef struct distribution_s distribution_t;

/* function that create new distribution structure and initialize buckets using linear function */
distribution_t *distribution_new_linear(size_t num_buckets, double size);
/* function that create new distribution structure and initialize buckets using exponential function */
distribution_t *distribution_new_exponential(size_t num_buckets, double factor);
/* function that create new distribution structure and initialize buckets using custom buckets sizes given by the user */
distribution_t* distribution_new_custom(size_t num_buckets, double *custom_buckets_sizes);

/* function for updating the buckets */
void distribution_update(distribution_t *d, double gauge);

/* function  for getting the percentile */
double distribution_percentile(distribution_t *d, double percent);

/* function that calculates average of gauges */
double distribution_average(distribution_t *d);

/* function that do the clone of distribution structure */
distribution_t* distribution_clone(distribution_t *d);

/* function that do clean up and free all the memory */
void distribution_destroy(distribution_t *d);

#endif // DISTRIBUTION_H
