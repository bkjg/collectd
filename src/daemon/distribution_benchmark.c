#include "collectd.h"
#include "distribution.h"

void run_distribution_new_linear(int max_size, uint64_t *elapsed_time) {
  distribution_t *dist_linear[max_size];
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    dist_linear[i] = distribution_new_linear(50, 124.543);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);

  for (int i = 0; i < max_size; ++i) {
    distribution_destroy(dist_linear[i]);
  }
}

void run_distribution_new_exponential(int max_size, uint64_t *elapsed_time) {
  distribution_t *dist_exponential[max_size];
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    dist_exponential[i] = distribution_new_exponential(50, 1.24543, 9);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);

  for (int i = 0; i < max_size; ++i) {
    distribution_destroy(dist_exponential[i]);
  }
}

void run_distribution_new_custom(int max_size, uint64_t *elapsed_time) {
  distribution_t *dist_custom[max_size];
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < max_size; ++i) {
    dist_custom[i] = distribution_new_custom(
        50,
        (double[]){755.384227,    1904.461413,   4211.399562,   8731.514392,
                   9632.999741,   11571.079506,  14857.476580,  23354.050924,
                   24910.147702,  24923.471081,  29928.720396,  29954.806259,
                   31151.986212,  34484.854599,  37304.902522,  38743.259528,
                   39946.535399,  42313.083732,  42664.653045,  42853.862497,
                   44799.530462,  50899.834602,  55688.641054,  58185.986540,
                   58588.067283,  60753.190469,  61514.099602,  61523.755980,
                   62167.505244,  69937.349131,  72081.568033,  72278.342695,
                   77941.730677,  78697.820813,  79006.794968,  80964.618037,
                   83871.189686,  84848.958981,  88158.735096,  88901.947690,
                   96060.648686,  97358.065199,  98299.644175,  99763.703514,
                   101239.978394, 101265.988319, 102885.914889, 104214.454458,
                   107902.630901, 110894.526514});
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  *elapsed_time += (uint64_t)1e9 * (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec);

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

int main() {
  static const int MAX_SIZE = 100000;
  static const int MAX_TURNS = 100;
  static const int NUM_DISTS = 3;

  uint64_t *elapsed_time_new_linear = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_new_exponential = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_new_custom = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_update = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_percentile = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_average = calloc(1, sizeof(uint64_t));
  uint64_t *elapsed_time_clone = calloc(1, sizeof(uint64_t));

  distribution_t *dists[NUM_DISTS];
  dists[0] = distribution_new_linear(50, 124.543);
  dists[1] = distribution_new_exponential(50, 1.24543, 9);
  dists[2] = distribution_new_custom(
      50, (double[]){755.384227,    1904.461413,   4211.399562,   8731.514392,
                     9632.999741,   11571.079506,  14857.476580,  23354.050924,
                     24910.147702,  24923.471081,  29928.720396,  29954.806259,
                     31151.986212,  34484.854599,  37304.902522,  38743.259528,
                     39946.535399,  42313.083732,  42664.653045,  42853.862497,
                     44799.530462,  50899.834602,  55688.641054,  58185.986540,
                     58588.067283,  60753.190469,  61514.099602,  61523.755980,
                     62167.505244,  69937.349131,  72081.568033,  72278.342695,
                     77941.730677,  78697.820813,  79006.794968,  80964.618037,
                     83871.189686,  84848.958981,  88158.735096,  88901.947690,
                     96060.648686,  97358.065199,  98299.644175,  99763.703514,
                     101239.978394, 101265.988319, 102885.914889, 104214.454458,
                     107902.630901, 110894.526514});

  for (int i = 0; i < MAX_TURNS; ++i) {
    run_distribution_new_linear(MAX_SIZE, elapsed_time_new_linear);
    run_distribution_new_exponential(MAX_SIZE, elapsed_time_new_exponential);
    run_distribution_new_custom(MAX_SIZE, elapsed_time_new_custom);
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

  return 0;
}