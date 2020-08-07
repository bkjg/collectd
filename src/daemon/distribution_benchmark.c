#include "collectd.h"
#include "distribution.h"

void run_distribution_mixed(distribution_t *d, int max_size,
                            uint64_t *elapsed_time) {
  if (d == NULL) {
    return;
  }

  struct timespec start, end;

  double updates[max_size * 9 + 1];
  double percents[max_size];

  for (int i = 0; i < max_size; ++i) {
    for (int j = 0; j < max_size; ++j) {
      updates[i * max_size + j] =
          (rand() / (double)RAND_MAX) + (rand() % (int)1e6);
    }

    percents[i] = (rand() / (double)RAND_MAX) + (rand() % 100);
  }
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int j = 0; j < max_size; ++j) {
    for (int i = 0; i < 9; ++i) {
      distribution_update(d, updates[j * max_size + i]);
    }
    distribution_percentile(d, percents[j]);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);
}

void run_distribution_update(distribution_t *d, int max_size,
                             uint64_t *elapsed_time) {
  if (d == NULL) {
    return;
  }

  struct timespec start, end;

  double updates[max_size];

  for (int i = 0; i < max_size; ++i) {
    updates[i] = (rand() / (double)RAND_MAX) + (rand() % (int)1e6);
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    distribution_update(d, updates[i]);
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

  struct timespec start, end;

  double percents[max_size];

  for (int i = 0; i < max_size; ++i) {
    percents[i] = (rand() / (double)RAND_MAX) + (rand() % 100);
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    distribution_percentile(d, percents[i]);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);
}

int main(int argc, char *argv[]) {
  static const int MAX_SIZE = 2000000;
  static const int MAX_TURNS = 10;
  static const int NUM_DISTS = 3;

  uint64_t *elapsed_time_update = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_percentile = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_mixed = calloc(1, sizeof(uint64_t));

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
    run_distribution_update(dists[i % NUM_DISTS], MAX_SIZE,
                            elapsed_time_update);
    run_distribution_percentile(dists[i % NUM_DISTS], MAX_SIZE,
                                elapsed_time_percentile);
    run_distribution_mixed(dists[i % NUM_DISTS], MAX_SIZE / 10,
                           elapsed_time_mixed);
  }

  printf("%d,%lf,%lf,%lf\n", num_buckets, (double)*elapsed_time_update / 1e9,
         (double)*elapsed_time_percentile / 1e9,
         (double)*elapsed_time_mixed / 1e9);

  free(elapsed_time_update);
  free(elapsed_time_percentile);
  free(elapsed_time_mixed);
  for (int i = 0; i < NUM_DISTS; ++i) {
    distribution_destroy(dists[i]);
  }

  free(custom_values);
  return 0;
}
