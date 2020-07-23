#include "distribution.h"

typedef struct {
  double max_boundary;
  uint64_t counter;
} bucket_t;

struct distribution_s {
  bucket_t *buckets;
  size_t num_buckets;
};

bucket_t *bucket_new_linear(size_t num_buckets, double size) {
  bucket_t *buckets = (bucket_t *) calloc(num_buckets, sizeof(bucket_t));

  for(size_t i = 0; i < num_buckets; ++i) {
    buckets[i]->max_boundary = (i + 1) * size; 
  }

  buckets[num_buckets - 1] = INFINITY;

  return buckets;
}

bucket_t *bucket_new_exponential(size_t num_buckets, double factor) {
  return NULL;
}

distribution_t *distribution_new_linear(size_t num_buckets, double size) {
  distribution_t *d = (distribution_t *) calloc(1, sizeof(distribution_t));

  d->buckets = bucket_new_linear(num_buckets, size);
  d->num_buckets = num_buckets;

  return d;
}

distribution_t *distribution_new_exponential(size_t num_buckets, double factor) {
  distribution_t *d = (distribution_t *) calloc(1, sizeof(distribution_t));

  d->buckets = bucket_new_exponential(num_buckets, factor);
  d->num_buckets = num_buckets;

  return d;
}

distribution_t *distribution_new_custom(size_t num_buckets, double *custom_buckets_sizes) {
  return NULL;
}

void distribution_update(distribution_t *d, double gauge) {

}

double distribution_percentile(distribution_t *d, double percent) {
  return 0;
}

double distribution_average(distribution_t *d) {
  return 0;
}

distribution_t distribution_clone(distribution_t *d) {
  return (distribution_t){0, 0};
}

void distribution_destroy(distribution_t *d) {

}
