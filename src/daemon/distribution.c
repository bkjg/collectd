#include <math.h>
#include <pthread.h>
#include "distribution.h"

typedef struct {
  double max_boundary;
  uint64_t counter;
} bucket_t;

struct distribution_s {
  bucket_t *buckets;
  double sum_gauges;
  size_t num_buckets;
  pthread_mutex_t mutex;
};

static bucket_t *bucket_new_linear(size_t num_buckets, double size) {
  bucket_t *buckets = (bucket_t *) calloc(num_buckets, sizeof(bucket_t));

  if (buckets != NULL) {
    for (size_t i = 0; i < num_buckets - 1; ++i) {
      buckets[i].max_boundary = (double)(i + 1) * size;
    }

    /* what if num_buckets is equal to zero? should we do assert or check it somehow? */
    buckets[num_buckets - 1].max_boundary = INFINITY;
  }

  return buckets;
}

static bucket_t *bucket_new_exponential(size_t num_buckets, double factor) {
  bucket_t *buckets = (bucket_t *) calloc(num_buckets, sizeof(bucket_t));

  if (buckets != NULL) {
    double multiplier = factor;

    for (size_t i = 0; i < num_buckets - 1; ++i) {
      buckets[i].max_boundary = multiplier;
      multiplier *= factor;
    }

    buckets[num_buckets - 1].max_boundary = INFINITY;
  }

  return buckets;
}

static bucket_t *bucket_new_custom(size_t num_buckets, const double *custom_buckets_sizes) {
  bucket_t *buckets = (bucket_t *) calloc(num_buckets, sizeof(bucket_t));

  if (buckets != NULL) {
    double ptr = 0.0;

    for (size_t i = 0; i < num_buckets - 1; ++i) {
      ptr += custom_buckets_sizes[i];
      buckets[i].max_boundary = ptr;
    }

    buckets[num_buckets - 1].max_boundary = INFINITY;
  }

  return buckets;
}

distribution_t *distribution_new_linear(size_t num_buckets, double size) {
  distribution_t *d = (distribution_t *) calloc(1, sizeof(distribution_t));

  if (d == NULL) {
    d->buckets = bucket_new_linear(num_buckets, size);
    d->num_buckets = num_buckets;
  }

  return d;
}

distribution_t *distribution_new_exponential(size_t num_buckets, double factor) {
  distribution_t *d = (distribution_t *) calloc(1, sizeof(distribution_t));

  if (d != NULL) {
    d->buckets = bucket_new_exponential(num_buckets, factor);
    d->num_buckets = num_buckets;
  }

  return d;
}

distribution_t *distribution_new_custom(size_t num_buckets, double *custom_buckets_sizes) {
  distribution_t *d = (distribution_t *) calloc(1, sizeof(distribution_t));

  if (d != NULL) {
    /* if bucket_new_custom will return NULL, should we return NULL for full function? */
    d->buckets = bucket_new_custom(num_buckets, custom_buckets_sizes);
    d->num_buckets = num_buckets;
  }

  return d;
}

/* TODO(bkjg): Make the code thread safe */
static void bucket_update(bucket_t *buckets, size_t num_buckets, double gauge) {
  /* can we assume that we won't receive null pointer as an argument? */
  /* can we assume num_buckets is greater than zero? */
  int ptr = (int)num_buckets - 1;

  while (buckets[ptr].max_boundary > gauge && ptr >= 0) {
    buckets[ptr].counter++;
    ptr--;
  }
}

void distribution_update(distribution_t *d, double gauge) {
  pthread_mutex_lock(&d->mutex);
  
  bucket_update(d->buckets, d->num_buckets, gauge);

  d->sum_gauges += gauge;
  pthread_mutex_unlock(&d->mutex);
}

static double find_percentile(bucket_t *buckets, size_t num_buckets, uint64_t quantity) {
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
  distribution_t *distribution = distribution_clone(d);
  uint64_t quantity = (percent / 100.0) * distribution->buckets[distribution->num_buckets - 1].counter;

  percent = find_percentile(distribution->buckets, distribution->num_buckets, quantity);

  free(distribution);
  return percent;
}

double distribution_average(distribution_t *d) {
  distribution_t *distribution = distribution_clone(d);

  double average = distribution->sum_gauges / (double) distribution->buckets[distribution->num_buckets - 1].counter;

  free(distribution);
  return average;
}

distribution_t* distribution_clone(distribution_t *d) {
  pthread_mutex_lock(&d->mutex);

  distribution_t *distribution = calloc(1, sizeof(distribution_t));

  if (distribution != NULL) {
    distribution->sum_gauges = d->sum_gauges;
    distribution->num_buckets = d->num_buckets;

    distribution->buckets = calloc(d->num_buckets, sizeof(bucket_t));

    memcpy(distribution->buckets, d->buckets, d->num_buckets * sizeof(bucket_t));
  }

  pthread_mutex_unlock(&d->mutex);

  return distribution;
}

void distribution_destroy(distribution_t *d) {
  /* should we check if we try to free the null pointer? */
  free(d->buckets);
  free(d);
}