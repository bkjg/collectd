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
    buckets[i].max_boundary = (i + 1) * size; 
  }

  buckets[num_buckets - 1].max_boundary = INFINITY;

  return buckets;
}

bucket_t *bucket_new_exponential(size_t num_buckets, double factor) {
  bucket_t *buckets = (bucket_t *) calloc(num_buckets, sizeof(bucket_t));

  double multiplier = factor;

  for (int i = 0; i < num_buckets; ++i) {
    buckets[i].max_boundary = multiplier;
    multiplier *= factor;
  }

  buckets[num_buckets - 1].max_boundary = INFINITY;

  return buckets;
}

bucket_t *bucket_new_custom(size_t num_buckets, double *custom_buckets_sizes) {
  bucket_t *buckets = (bucket_t *) calloc(num_buckets, sizeof(bucket_t));
  double ptr = 0.0;

  for (int i = 0; i < num_buckets - 1; ++i) {
    ptr += custom_buckets_sizes[i];
    buckets[i].max_boundary = ptr;
  }

  buckets[num_buckets -1].max_boundary = INFINITY;

  return buckets;
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
  distribution_t *d = (distribution_t *) calloc(1, sizeof(distribution_t));

  d->buckets = bucket_new_custom(num_buckets, custom_buckets_sizes);
  d->num_buckets = num_buckets;

  return d;
}

void bucket_update(bucket_t *buckets, size_t num_buckets, double gauge) {
  size_t ptr = num_buckets - 1;

  while (buckets[ptr].max_boundary > gauge && ptr >= 0) {
    buckets[ptr].counter++;
    ptr--;
  }
}

void distribution_update(distribution_t *d, double gauge) {
  bucket_update(d->buckets, d->num_buckets, gauge);
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
  free(d->buckets);
  free(d);
}
