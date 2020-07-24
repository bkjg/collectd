#include "distribution.h"

typedef struct {
  double max_boundary;
  uint64_t counter;
} bucket_t;

struct distribution_s {
  bucket_t *buckets;
  double sum_gauges;
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

/* TODO(bkjg): Make the code thread safe */
void bucket_update(bucket_t *buckets, size_t num_buckets, double gauge) {
  size_t ptr = num_buckets - 1;

  while (buckets[ptr].max_boundary > gauge && ptr >= 0) {
    buckets[ptr].counter++;
    ptr--;
  }
}

void distribution_update(distribution_t *d, double gauge) {
  static pthread_mutex_t mutex;
  pthread_mutex_lock(&mutex);
  
  bucket_update(d->buckets, d->num_buckets, gauge);

  d->sum_gauges += gauge;

  pthread_mutex_unlock(&mutex);
}

double find_percentile(bucket_t *buckets, size_t num_buckets, uint64_t quantity) {
  size_t left = 0;
  size_t right = num_buckets - 1;
  size_t middle;

  while (left < right) {
    middle = (left + right) / 2;
    
    if (buckets[middle].counter >= quantity) {
      left = middle;
    } else {
      right = middle - 1;
    }
  }

  return buckets[left].max_boundary;
}

double distribution_percentile(distribution_t *d, double percent) {
  uint64_t quantity = (percent / 100.0) * d->buckets[d->num_buckets - 1].counter;

  return find_percentile(d->buckets, d->num_buckets, quantity);
}

double distribution_average(distribution_t *d) {
  return d->sum_gauges / (double) d->buckets[d->num_buckets - 1].counter;
}

distribution_t distribution_clone(distribution_t *d) {
  return (distribution_t){0, 0};
}

void distribution_destroy(distribution_t *d) {
  free(d->buckets);
  free(d);
}
