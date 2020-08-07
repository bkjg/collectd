#include "collectd.h"
#include "distribution.h"

void run_distribution_new_linear(int num_buckets, int max_size,
                                 uint64_t *elapsed_time) {
  distribution_t *dist_linear[max_size];
  struct timespec start, end;

  double diff = (double)(rand() / (double)RAND_MAX) + (rand() % (int)1e6) + 1.0;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    dist_linear[i] = distribution_new_linear(num_buckets, diff);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);

  for (int i = 0; i < max_size; ++i) {
    distribution_destroy(dist_linear[i]);
  }
}

void run_distribution_new_exponential(int num_buckets, int max_size,
                                      uint64_t *elapsed_time) {
  distribution_t *dist_exponential[max_size];
  struct timespec start, end;

  double base = (double)(rand() / (double)RAND_MAX) + (rand() % 3);
  double factor =
      (double)(rand() / (double)RAND_MAX) + (rand() % (int)1e6) + 1.0;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    dist_exponential[i] =
        distribution_new_exponential(num_buckets, base, factor);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);

  for (int i = 0; i < max_size; ++i) {
    distribution_destroy(dist_exponential[i]);
  }
}

void run_distribution_new_custom(int num_buckets, int max_size,
                                 uint64_t *elapsed_time) {
  distribution_t *dist_custom[max_size];
  struct timespec start, end;

  double *custom_values = calloc(num_buckets - 1, sizeof(double));

  srand(time(NULL));
  double prev = 0;
  for (int i = 0; i < num_buckets - 1; ++i) {
    custom_values[i] =
        (rand() / (double)RAND_MAX) + (rand() % (int)1e6 + prev) + 1;
    prev = custom_values[i];
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    dist_custom[i] = distribution_new_custom(num_buckets, custom_values);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);

  free(custom_values);
  for (int i = 0; i < max_size; ++i) {
    distribution_destroy(dist_custom[i]);
  }
}

void run_distribution_update(distribution_t *d, int max_size,
                             uint64_t *elapsed_time) {
  if (d == NULL) {
    return;
  }

  double gauges[max_size];
  srand(time(NULL));

  for (int i = 0; i < max_size; ++i) {
    gauges[i] = (rand() / (double)RAND_MAX) + (rand() % (int)1e6);
  }

  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    distribution_update(d, gauges[i]);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);
}

void run_distribution_percentile(distribution_t *d, int max_size,
                                 uint64_t *elapsed_time) {
  if (d == NULL) {
    return;
  }

  double percents[max_size];
  srand(time(NULL));

  for (int i = 0; i < max_size; ++i) {
    percents[i] = (rand() / (double)RAND_MAX) + (rand() % (int)100);
  }

  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    distribution_percentile(d, percents[i]);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);
}

void run_distribution_average(distribution_t *d, int max_size,
                              uint64_t *elapsed_time) {
  if (d == NULL) {
    return;
  }

  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    distribution_average(d);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);
}

void run_distribution_clone(distribution_t *d, int max_size,
                            uint64_t *elapsed_time) {
  if (d == NULL) {
    return;
  }

  distribution_t *dists[max_size];
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    dists[i] = distribution_clone(d);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);

  for (int i = 0; i < max_size; ++i) {
    distribution_destroy(dists[i]);
  }
}

int main(int argc, char *argv[]) {
  static const int MAX_SIZE = 1000000;
  static const int MAX_TURNS = 100;
  static const int NUM_DISTS = 3;

  uint64_t *elapsed_time_new_linear = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_new_exponential = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_new_custom = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_update = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_percentile = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_average = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_clone = calloc(1, sizeof(uint64_t));

  if (argc < 2) {
    fprintf(stderr, "Usage %s NUM_BUCKETS\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int num_buckets = atoi(argv[1]);

  double *custom_values = calloc(num_buckets - 1, sizeof(double));

  srand(time(NULL));
  double prev = 0;
  for (int i = 0; i < num_buckets - 1; ++i) {
    custom_values[i] =
        (rand() / (double)RAND_MAX) + (rand() % (int)1e6 + prev) + 1;
    prev = custom_values[i];
  }

  distribution_t *dists[NUM_DISTS];
  dists[0] = distribution_new_linear(num_buckets, 124.543);
  dists[1] = distribution_new_exponential(num_buckets, 1.24543, 9);
  dists[2] = distribution_new_custom(num_buckets - 1, custom_values);

  for (int i = 0; i < MAX_TURNS; ++i) {
    run_distribution_new_linear(num_buckets, MAX_SIZE, elapsed_time_new_linear);
    run_distribution_new_exponential(num_buckets, MAX_SIZE,
                                     elapsed_time_new_exponential);
    run_distribution_new_custom(num_buckets, MAX_SIZE, elapsed_time_new_custom);
    run_distribution_update(dists[i % NUM_DISTS], MAX_SIZE,
                            elapsed_time_update);
    run_distribution_percentile(dists[i % NUM_DISTS], MAX_SIZE,
                                elapsed_time_percentile);
    run_distribution_average(dists[i % NUM_DISTS], MAX_SIZE,
                             elapsed_time_average);
    run_distribution_clone(dists[i % NUM_DISTS], MAX_SIZE, elapsed_time_clone);
  }

  printf("distribution_new_linear: %lf\n",
         (double)*elapsed_time_new_linear / 1e9);
  printf("distribution_new_exponential: %lf\n",
         (double)*elapsed_time_new_exponential / 1e9);
  printf("distribution_new_custom: %lf\n",
         (double)*elapsed_time_new_custom / 1e9);
  printf("distribution_update: %lf\n", (double)*elapsed_time_update / 1e9);
  printf("distribution_percentile: %lf\n",
         (double)*elapsed_time_percentile / 1e9);
  printf("distribution_average: %lf\n", (double)*elapsed_time_average / 1e9);
  printf("distribution_clone: %lf\n", (double)*elapsed_time_clone / 1e9);

  free(elapsed_time_new_linear);
  free(elapsed_time_new_exponential);
  free(elapsed_time_new_custom);
  free(elapsed_time_update);
  free(elapsed_time_percentile);
  free(elapsed_time_average);
  free(elapsed_time_clone);

  for (int i = 0; i < NUM_DISTS; ++i) {
    distribution_destroy(dists[i]);
  }

  free(custom_values);
  return 0;
}
