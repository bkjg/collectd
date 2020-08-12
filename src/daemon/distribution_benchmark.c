#include "collectd.h"
#include "distribution.h"

#define MAX_SIZE 2000000

double updates[MAX_SIZE];
double percents[MAX_SIZE];
/* variable to which we save the return value from the measured functions to make sure that compiler won't delete these lines */
volatile double result;

void run_distribution_mixed(distribution_t *d, int max_size,
                            uint64_t *elapsed_time) {
  if (d == NULL || elapsed_time == NULL) {
    return;
  }

  struct timespec start, end;

  for (int i = 0; i < max_size; ++i) {
    for (int j = 0; j < 9; ++j) {
      updates[i * 9 + j] = (rand() / (double)RAND_MAX) + (rand() % (int)1e6);
    }

    percents[i] = (rand() / (double)RAND_MAX) + (rand() % 100);
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int j = 0; j < max_size; ++j) {
    for (int i = 0; i < 9; ++i) {
      result = distribution_update(d, updates[j * 9 + i]);
    }
    result = distribution_percentile(d, percents[j]);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);
}

void run_distribution_update(distribution_t *d, int max_size,
                             uint64_t *elapsed_time) {
  if (d == NULL || elapsed_time == NULL) {
    return;
  }

  struct timespec start, end;

  for (int i = 0; i < max_size; ++i) {
    updates[i] = (rand() / (double)RAND_MAX) + (rand() % (int)1e6);
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    result = distribution_update(d, updates[i]);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);
}

void run_distribution_percentile(distribution_t *d, int max_size,
                                 uint64_t *elapsed_time) {
  if (d == NULL || elapsed_time == NULL) {
    return;
  }

  struct timespec start, end;

  for (int i = 0; i < max_size; ++i) {
    percents[i] = (rand() / (double)RAND_MAX) + (rand() % 100);
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    result = distribution_percentile(d, percents[i]);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);
}

int main(int argc, char *argv[]) {
  static const int MAX_TURNS = 10;
  static const int NUM_DISTS = 3;

  uint64_t elapsed_time_update = 0;
  uint64_t elapsed_time_percentile = 0;
  uint64_t elapsed_time_mixed = 0;

  if (argc < 2) {
    fprintf(stderr, "Usage %s NUM_BUCKETS\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int num_buckets = atoi(argv[1]);

  if (num_buckets <= 0) {
    fprintf(stderr, "num_buckets have to be greater than zero!\n");
    fprintf(stderr, "Usage %s NUM_BUCKETS\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* if num_buckets will be 1, then calloc can return a null or a valid pointer that can be freed
   * however it won't be any issue with that because the distribution_new_custom function
   * can handle that */
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

  for (int i = 0; i < NUM_DISTS; ++i) {
    if (dists[i] == NULL) {
      fprintf(stderr, "Creating distribution failed! Exited!\n");

      for (int j = 0; j < i; ++j) {
        distribution_destroy(dists[j]);
      }

      for (int j = i + 1; j < NUM_DISTS; ++j) {
        distribution_destroy(dists[j]);
      }

      free(custom_values);
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < MAX_TURNS; ++i) {
    run_distribution_update(dists[i % NUM_DISTS], MAX_SIZE,
                            &elapsed_time_update);
    run_distribution_percentile(dists[i % NUM_DISTS], MAX_SIZE,
                                &elapsed_time_percentile);
    run_distribution_mixed(dists[i % NUM_DISTS], (MAX_SIZE / 10),
                           &elapsed_time_mixed);
  }

  printf("%d,%lf,%lf,%lf\n", num_buckets, (double)elapsed_time_update / 1e9,
         (double)elapsed_time_percentile / 1e9,
         (double)elapsed_time_mixed / 1e9);

  for (int i = 0; i < NUM_DISTS; ++i) {
    distribution_destroy(dists[i]);
  }

  free(custom_values);
  return 0;
}
