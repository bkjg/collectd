#include "distribution.h"
#include <math.h>
#include <pthread.h>

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
  bucket_t *buckets = (bucket_t *)calloc(num_buckets, sizeof(bucket_t));

  if (buckets == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < num_buckets - 1; ++i) {
    buckets[i].max_boundary = (double)(i + 1) * size;
  }

  buckets[num_buckets - 1].max_boundary = INFINITY;

  return buckets;
}

static bucket_t *bucket_new_exponential(size_t num_buckets, double initial_size,
                                        double factor) {
  bucket_t *buckets = (bucket_t *)calloc(num_buckets, sizeof(bucket_t));

  if (buckets == NULL) {
    return NULL;
  }

  double multiplier = initial_size;

  for (size_t i = 0; i < num_buckets - 1; ++i) {
    buckets[i].max_boundary = factor * multiplier;
    multiplier *= initial_size;
  }

  buckets[num_buckets - 1].max_boundary = INFINITY;

  return buckets;
}

static bucket_t *bucket_new_custom(size_t num_buckets,
                                   const double *custom_buckets_boundaries) {
  bucket_t *buckets = (bucket_t *)calloc(num_buckets, sizeof(bucket_t));

  if (buckets == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < num_buckets - 1; ++i) {
    buckets[i].max_boundary = custom_buckets_boundaries[i];
  }

  buckets[num_buckets - 1].max_boundary = INFINITY;

  return buckets;
}

distribution_t *distribution_new_linear(size_t num_buckets, double size) {
  distribution_t *d = (distribution_t *)calloc(1, sizeof(distribution_t));

  if (d == NULL) {
    return NULL;
  }

  d->buckets = bucket_new_linear(num_buckets, size);

  if (d->buckets == NULL) {
    free(d);
    return NULL;
  }

  d->num_buckets = num_buckets;
  pthread_mutex_init(&d->mutex, NULL);

  return d;
}

distribution_t *distribution_new_exponential(size_t num_buckets,
                                             double initial_size,
                                             double factor) {
  distribution_t *d = (distribution_t *)calloc(1, sizeof(distribution_t));

  if (d == NULL) {
    return NULL;
  }

  d->buckets = bucket_new_exponential(num_buckets, initial_size, factor);

  if (d->buckets == NULL) {
    free(d);
    return NULL;
  }

  d->num_buckets = num_buckets;
  pthread_mutex_init(&d->mutex, NULL);

  return d;
}

distribution_t *distribution_new_custom(size_t num_buckets,
                                        double *custom_buckets_boundaries) {
  distribution_t *d = (distribution_t *)calloc(1, sizeof(distribution_t));

  if (d == NULL) {
    return NULL;
  }

  d->buckets = bucket_new_custom(num_buckets, custom_buckets_boundaries);

  if (d->buckets == NULL) {
    free(d);
    return NULL;
  }

  d->num_buckets = num_buckets;
  pthread_mutex_init(&d->mutex, NULL);

  return d;
}

static void bucket_update(bucket_t *buckets, size_t num_buckets, double gauge) {
  int ptr = (int)num_buckets - 1;

  while (buckets[ptr].max_boundary > gauge && ptr >= 0) {
    buckets[ptr].counter++;
    ptr--;
  }
}

void distribution_update(distribution_t *d, double gauge) {
  if (d == NULL) {
    errno = EINVAL;
    return;
  }

  pthread_mutex_lock(&d->mutex);

  bucket_update(d->buckets, d->num_buckets, gauge);

  d->sum_gauges += gauge;
  pthread_mutex_unlock(&d->mutex);
}

static double find_percentile(bucket_t *buckets, size_t num_buckets,
                              uint64_t quantity) {
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
  if (d == NULL) {
    errno = EINVAL;
    return -1;
  }

  pthread_mutex_lock(&d->mutex);

  uint64_t quantity =
      (percent / 100.0) * d->buckets[d->num_buckets - 1].counter;

  percent = find_percentile(d->buckets, d->num_buckets, quantity);

  pthread_mutex_unlock(&d->mutex);
  return percent;
}

double distribution_average(distribution_t *d) {
  if (d == NULL) {
    errno = EINVAL;
    return -1;
  }

  pthread_mutex_lock(&d->mutex);

  double average =
      d->sum_gauges / (double)d->buckets[d->num_buckets - 1].counter;

  pthread_mutex_unlock(&d->mutex);

  return average;
}

distribution_t *distribution_clone(distribution_t *d) {
  if (d == NULL) {
    errno = EINVAL;
    return NULL;
  }

  distribution_t *distribution = calloc(1, sizeof(distribution_t));

  if (distribution == NULL) {
    return NULL;
  }

  pthread_mutex_lock(&d->mutex);

  distribution->sum_gauges = d->sum_gauges;
  distribution->num_buckets = d->num_buckets;

  distribution->buckets = calloc(d->num_buckets, sizeof(bucket_t));

  if (distribution->buckets == NULL) {
    free(distribution);
    pthread_mutex_unlock(&d->mutex);
    return NULL;
  }

  memcpy(distribution->buckets, d->buckets, d->num_buckets * sizeof(bucket_t));

  pthread_mutex_init(&distribution->mutex, NULL);

  pthread_mutex_unlock(&d->mutex);

  return distribution;
}

void distribution_destroy(distribution_t *d) {
  if (d == NULL) {
    return;
  }

  pthread_mutex_destroy(&d->mutex);
  free(d->buckets);
  free(d);
}
