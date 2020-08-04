#include "collectd.h"

#include "distribution.c"
#include "distribution.h"
#include "testing.h"

/* TODO(bkjg): add checking errno */
/* TODO(bkjg): add checking sum_gauges and counters in buckets */
/* TODO(bkjg): add checking if called function changed the content of
 * distribution_t structure */

double *array_new_linear(size_t size, double diff) {
  if (size == 0 || diff <= 0) {
    return NULL;
  }

  double *arr = calloc(size, sizeof(double));

  if (arr == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < size - 1; ++i) {
    arr[i] = (double)(i + 1) * diff;
  }

  arr[size - 1] = INFINITY;

  return arr;
}

double *array_new_exponential(size_t size, double base, double factor) {
  double *arr = calloc(size, sizeof(double));

  if (arr == NULL) {
    return NULL;
  }

  arr[0] = factor;
  for (size_t i = 1; i < size - 1; ++i) {
    arr[i] = arr[i - 1] * base;
  }

  arr[size - 1] = INFINITY;

  return arr;
}

DEF_TEST(distribution_new_linear) {
  struct {
    size_t num_buckets;
    double size;
    double *want_get;
  } cases[] = {{
                   .num_buckets = 0,
                   .want_get = NULL,
               },
               {
                   .num_buckets = 10,
                   .size = -5,
                   .want_get = NULL,
               },
               {.num_buckets = 8, .size = 0, .want_get = NULL},
               {
                   .num_buckets = 10,
                   .size = 2,
                   .want_get = array_new_linear(10, 2),
               },
               {
                   .num_buckets = 20,
                   .size = 0.67,
                   .want_get = array_new_linear(20, 0.67),
               },
               {
                   .num_buckets = 48,
                   .size = 8.259,
                   .want_get = array_new_linear(48, 8.259),
               }};

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    distribution_t *d =
        distribution_new_linear(cases[i].num_buckets, cases[i].size);

    if (cases[i].want_get == NULL) {
      EXPECT_EQ_PTR(cases[i].want_get, d);
    } else {
      CHECK_NOT_NULL(d);
      EXPECT_EQ_UINT64(cases[i].num_buckets, distribution_get_num_buckets(d));

      double *boundaries = distribution_get_buckets_boundaries(d);
      uint64_t *counters = distribution_get_buckets_counters(d);

      for (size_t j = 0; j < cases[i].num_buckets; ++j) {
        EXPECT_EQ_DOUBLE(cases[i].want_get[j], boundaries[j]);
        EXPECT_EQ_UINT64(0, counters[j]);
      }

      EXPECT_EQ_DOUBLE(0, distribution_get_sum_gauges(d));
      free(boundaries);
      free(counters);
    }

    free(cases[i].want_get);
    distribution_destroy(d);
  }

  return 0;
}

DEF_TEST(distribution_new_exponential) {
  struct {
    size_t num_buckets;
    double initial_size;
    double factor;
    double *want_get;
  } cases[] = {
      {
          .num_buckets = 0,
          .want_get = NULL,
      },
      {
          .num_buckets = 10,
          .initial_size = -52,
          .want_get = NULL,
      },
      {.num_buckets = 8, .initial_size = 0, .want_get = NULL},
      {.num_buckets = 12, .initial_size = 2, .factor = 0, .want_get = NULL},
      {.num_buckets = 33, .initial_size = 7, .factor = -5, .want_get = NULL},
      {
          .num_buckets = 6,
          .initial_size = 2,
          .factor = 3,
          .want_get = array_new_exponential(6, 2, 3),
      },
      {
          .num_buckets = 10,
          .initial_size = 5,
          .factor = 6.75,
          .want_get = array_new_exponential(10, 5, 6.75),
      },
      {
          .num_buckets = 26,
          .initial_size = 1.01,
          .factor = 4.64,
          .want_get = array_new_exponential(26, 1.01, 4.64),
      }};

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    distribution_t *d = distribution_new_exponential(
        cases[i].num_buckets, cases[i].initial_size, cases[i].factor);

    if (cases[i].want_get == NULL) {
      EXPECT_EQ_PTR(cases[i].want_get, d);
    } else {
      CHECK_NOT_NULL(d);

      EXPECT_EQ_UINT64(cases[i].num_buckets, distribution_get_num_buckets(d));
      double *boundaries = distribution_get_buckets_boundaries(d);
      uint64_t *counters = distribution_get_buckets_counters(d);

      for (size_t j = 0; j < cases[i].num_buckets; ++j) {
        EXPECT_EQ_DOUBLE(cases[i].want_get[j], boundaries[j]);
        EXPECT_EQ_UINT64(0, counters[j]);
      }

      EXPECT_EQ_DOUBLE(0, distribution_get_sum_gauges(d));
      free(boundaries);
      free(counters);
    }

    free(cases[i].want_get);
    distribution_destroy(d);
  }
  return 0;
}

DEF_TEST(distribution_new_custom) {
  struct {
    size_t num_boundaries;
    double *given_boundaries;
    double *want_get;
  } cases[] = {
      {
          .num_boundaries = 0,
          .want_get = (double[]){INFINITY},
      },
      {
          .num_boundaries = 5,
          .given_boundaries = (double[]){5, 4, 6, 7, 8},
          .want_get = NULL,
      },
      {.num_boundaries = 4,
       .given_boundaries = (double[]){-2, 4, 5, 6},
       .want_get = NULL},
      {.num_boundaries = 7,
       .given_boundaries =
           (double[]){1.23, 4.76, 6.324, 8.324, 9.342, 16.4234, 90.4234},
       .want_get = (double[]){1.23, 4.76, 6.324, 8.324, 9.342, 16.4234, 90.4234,
                              INFINITY}},
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    distribution_t *d = distribution_new_custom(cases[i].num_boundaries,
                                                cases[i].given_boundaries);

    if (cases[i].want_get == NULL) {
      EXPECT_EQ_PTR(cases[i].want_get, d);
    } else {
      CHECK_NOT_NULL(d);
      EXPECT_EQ_UINT64(cases[i].num_boundaries + 1,
                       distribution_get_num_buckets(d));
      double *boundaries = distribution_get_buckets_boundaries(d);
      uint64_t *counters = distribution_get_buckets_counters(d);

      for (size_t j = 0; j < cases[i].num_boundaries + 1; ++j) {
        EXPECT_EQ_DOUBLE(cases[i].want_get[j], boundaries[j]);
        EXPECT_EQ_UINT64(0, counters[j]);
      }

      EXPECT_EQ_DOUBLE(0, distribution_get_sum_gauges(d));
      free(boundaries);
      free(counters);
    }

    distribution_destroy(d);
  }

  return 0;
}

DEF_TEST(distribution_update) {
  distribution_t *dist_test1 = distribution_new_linear(15, 34.834);
  distribution_t *dist_test2 = distribution_new_exponential(25, 1.673, 6.8);
  distribution_t *dist_test3 = distribution_new_custom(
      18, (double[]){1, 5, 25, 125, 625, 1000, 1001, 1005, 1025, 1125, 1625,
                     2000, 2001, 2005, 2025, 2125, 2625, 3000});
  struct {
    size_t num_buckets;
    uint64_t *counters;
    distribution_t *d;
    double *gauges;
    int num_queries;
    int *status_codes;
    double want_sum;
    double want_percentile;
    double want_average;
    double percent;
  } cases[] = {
      {
          .num_buckets = 0,
          .num_queries = 1,
          .d = NULL,
          .status_codes = (int[]){EXIT_FAILURE},
          .gauges = (double[]){54.6},
          .want_sum = NAN,
          .want_average = NAN,
          .want_percentile = NAN,
          .percent = 15.9,
      },
      {
          .num_buckets = 15,
          .d = dist_test1,
          .num_queries = 16,
          .gauges = (double[]){5, 1, 6.74, 23.54, 52.6435, 23.523, 6554.534,
                               87.543, 135.34, 280.43, 100.624, 40.465, -78.213,
                               -90.423, -1423.423, -9.432},
          .status_codes =
              (int[]){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, EXIT_FAILURE,
                      EXIT_FAILURE, EXIT_FAILURE, EXIT_FAILURE},
          .want_sum = 7311.3825,
          .want_average = 609.281875,
          .want_percentile = 34.834,
          .percent = 15.67,
          .counters = (uint64_t[]){5, 7, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11,
                                   11, 11, 12},
      },
      {
          .num_buckets = 25,
          .d = dist_test2,
          .gauges = (double[]){10.45,        26.43,           98.84,
                               1067.27,      905.326,         46.7242,
                               205.653,      542.876,         24543.543,
                               35262.6345,   64262.24624,     8753.635,
                               26.264,       675645.346346,   764.436,
                               2345.56,      23456.789,       65543.6456,
                               10583.72023,  896496.6532,     738563.5723,
                               23562.534652, 5325927.253234,  5237452.523523,
                               462383.25235, 7936583.3734475, 9000,
                               6.8,          11.3764,         19.0327172},
          .num_queries = 30,
          .want_sum = 21544097.760940,
          .want_average = 718136.592031,
          .percent = 90.345,
          .want_percentile = 939540.606494,
          .status_codes =
              (int[]){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
          .counters =
              (uint64_t[]){0,  2,  3,  6,  7,  7,  8,  9,  9,  10, 13, 13, 14,
                           14, 16, 17, 20, 21, 23, 23, 23, 23, 24, 27, 30},
      },
      {
          .num_buckets = 19,
          .d = dist_test3,
          .gauges = (double[]){1, 5.43, 6.42626, 625, 625.1, 624.999999, 1000,
                               999.999999, 0, 999999, 1001, -1},
          .status_codes =
              (int[]){0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, EXIT_FAILURE},
          .num_queries = 12,
          .percent = 0,
          .want_percentile = 1,
          .want_average = 91353.450569,
          .want_sum = 1004887.956258,
          .counters = (uint64_t[]){1, 2, 4, 4, 5, 8, 9, 10, 10, 10, 10, 10, 10,
                                   10, 10, 10, 10, 10, 11},
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    for (int j = 0; j < cases[i].num_queries; ++j) {
      EXPECT_EQ_INT(cases[i].status_codes[j],
                    distribution_update(cases[i].d, cases[i].gauges[j]));
    }

    EXPECT_EQ_UINT64(cases[i].num_buckets,
                     distribution_get_num_buckets(cases[i].d));

    uint64_t *counters = distribution_get_buckets_counters(cases[i].d);

    if (counters != NULL) {
      for (size_t j = 0; j < cases[i].num_buckets; ++j) {
        EXPECT_EQ_UINT64(cases[i].counters[j], counters[j]);
      }
      free(counters);
    }

    char buffer[256];
    double sum = distribution_get_sum_gauges(cases[i].d);
    double average = distribution_average(cases[i].d);
    double percentile = distribution_percentile(cases[i].d, cases[i].percent);

    snprintf(buffer, 256, "%.6lf", sum);
    sscanf(buffer, "%lf", &sum);

    snprintf(buffer, 256, "%.6lf", average);
    sscanf(buffer, "%lf", &average);

    snprintf(buffer, 256, "%.6lf", percentile);
    sscanf(buffer, "%lf", &percentile);

    EXPECT_EQ_DOUBLE(cases[i].want_sum, sum);
    EXPECT_EQ_DOUBLE(cases[i].want_average, average);
    EXPECT_EQ_DOUBLE(cases[i].want_percentile, percentile);

    distribution_destroy(cases[i].d);
  }

  return 0;
}

DEF_TEST(distribution_clone) {
  /* e.g. 4.576, 6.432, 8.432, 10.423, 11.54,
   * 20.423, 29.312
   * 40.231, 42.423, 44.432
   * 50.12, 53.32, 54.543, 57.423, 58.423, 59.2141
   * 80.342, 90.4235456, 100.3425, 150.34 */
  bucket_t case1_buckets[] = {
      {
          .max_boundary = 15,
          .counter = 5,
      },
      {
          .max_boundary = 30,
          .counter = 7,
      },
      {
          .max_boundary = 45,
          .counter = 10,
      },
      {
          .max_boundary = 60,
          .counter = 16,
      },
      {
          .max_boundary = INFINITY,
          .counter = 20,
      },
  };
  distribution_t case1_distribution = {
      .buckets = case1_buckets,
      .num_buckets = 5,
      .sum_gauges = 972.7151456,
  };

  struct {
    distribution_t *input_dist;
    distribution_t *want_get;
  } cases[] = {
      {
          .input_dist = NULL,
          .want_get = NULL,
      },
      {
          .input_dist = &case1_distribution,
          .want_get = &case1_distribution,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    distribution_t *d = distribution_clone(cases[i].input_dist);

    if (cases[i].want_get == NULL) {
      EXPECT_EQ_PTR(cases[i].want_get, d);
    } else {
      CHECK_NOT_NULL(d);
      EXPECT_EQ_INT(1, distribution_check_equal(cases[i].want_get, d));
    }

    distribution_destroy(d);
  }

  return 0;
}

DEF_TEST(distribution_average) {
  /* TODO(bkjg): add test when sum_gauges is equal zero and when there is only
   * one gauge inside */
  /* e.g. 4.576, 6.432, 8.432, 10.423, 11.54,
   * 20.423, 29.312
   * 40.231, 42.423, 44.432
   * 50.12, 53.32, 54.543, 57.423, 58.423, 59.2141
   * 80.342, 90.4235456, 100.3425, 150.34 */
  bucket_t case1_buckets[] = {
      {
          .max_boundary = 15,
          .counter = 5,
      },
      {
          .max_boundary = 30,
          .counter = 7,
      },
      {
          .max_boundary = 45,
          .counter = 10,
      },
      {
          .max_boundary = 60,
          .counter = 16,
      },
      {
          .max_boundary = INFINITY,
          .counter = 20,
      },
  };
  distribution_t case1_distribution = {
      .buckets = case1_buckets,
      .num_buckets = 5,
      .sum_gauges = 972.7151456,
  };

  struct {
    distribution_t *input_dist;
    double want_get;
  } cases[] = {
      {
          .input_dist = NULL,
          .want_get = NAN,
      },
      {
          .input_dist = &case1_distribution,
          .want_get = 48.63575728,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    EXPECT_EQ_DOUBLE(cases[i].want_get,
                     distribution_average(cases[i].input_dist));
  }

  return 0;
}

DEF_TEST(distribution_percentile) {
  distribution_t *dist_test = distribution_new_linear(5, 15.0);

  struct {
    distribution_t *input_dist;
    double percent;
    double want_percentile;
    double *gauges;
    int num_queries;
    uint64_t *counters;
    int num_buckets;
    double sum_gauges;
  } cases[] = {
      {
          .input_dist = NULL,
          .want_percentile = NAN,
          .num_queries = 0,
          .num_buckets = 0,
          .sum_gauges = NAN,
      },
      {
          .input_dist = dist_test,
          .percent = -5,
          .want_percentile = NAN,
          .num_queries = 0,
          .num_buckets = 5,
          .counters = (uint64_t[]){0, 0, 0, 0, 0},
          .sum_gauges = 0,
      },
      {
          .num_queries = 4,
          .gauges = (double[]){4.576, 6.432, 8.432, 10.423},
          .percent = 110.9,
          .want_percentile = NAN,
          .num_buckets = 5,
          .counters = (uint64_t[]){4, 4, 4, 4, 4},
          .sum_gauges = 29.863,
      },
      {
          .num_queries = 1,
          .gauges = (double[]){90.4235456},
          .percent = -0.12,
          .want_percentile = NAN,
          .num_buckets = 5,
          .counters = (uint64_t[]){4, 4, 4, 4, 5},
          .sum_gauges = 120.286546,
      },
      {
          .percent = 5.67,
          .want_percentile = 15,
          .num_queries = 15,
          .num_buckets = 5,
          .gauges = (double[]){11.54, 20.423, 29.312, 40.231, 42.423, 44.432,
                               50.12, 53.32, 54.543, 57.423, 58.423, 59.2141,
                               80.342, 100.3425, 150.34},
          .counters = (uint64_t[]){5, 7, 10, 16, 20},
          .sum_gauges = 972.715146,
      },
      {
          .percent = 100,
          .want_percentile = INFINITY,
          .num_queries = 0,
          .num_buckets = 5,
          .sum_gauges = 972.715146,
          .counters = (uint64_t[]){5, 7, 10, 16, 20},
      }};

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    if (i > 0 && cases[i - 1].input_dist != NULL) {
      cases[i].input_dist = distribution_clone(cases[i - 1].input_dist);

      EXPECT_EQ_INT(1, distribution_check_equal(cases[i].input_dist,
                                                cases[i - 1].input_dist));
    }

    for (int j = 0; j < cases[i].num_queries; ++j) {
      // EXPECT_EQ_INT(cases[i].status_codes[j],
      distribution_update(cases[i].input_dist, cases[i].gauges[j]);
    }

    EXPECT_EQ_UINT64(cases[i].num_buckets,
                     distribution_get_num_buckets(cases[i].input_dist));

    uint64_t *counters = distribution_get_buckets_counters(cases[i].input_dist);

    if (counters != NULL) {
      for (size_t j = 0; j < cases[i].num_buckets; ++j) {
        EXPECT_EQ_UINT64(cases[i].counters[j], counters[j]);
      }
      free(counters);
    }

    char buffer[256];
    double sum = distribution_get_sum_gauges(cases[i].input_dist);
    // double average = distribution_average(cases[i].input_dist);
    double percentile =
        distribution_percentile(cases[i].input_dist, cases[i].percent);

    snprintf(buffer, 256, "%.6lf", sum);
    sscanf(buffer, "%lf", &sum);

    /*snprintf(buffer, 256, "%.6lf", average);
    sscanf(buffer, "%lf", &average);*/

    EXPECT_EQ_DOUBLE(cases[i].sum_gauges, sum);
    // EXPECT_EQ_DOUBLE(cases[i].want_average, average);
    EXPECT_EQ_DOUBLE(cases[i].want_percentile, percentile);
  }

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    distribution_destroy(cases[i].input_dist);
  }

  return 0;
}

DEF_TEST(distribution_get_num_buckets) {
  distribution_t *dist_test1 = distribution_new_linear(5, 15.0);
  distribution_t *dist_test2 = distribution_new_exponential(14, 1.5, 4);
  distribution_t *dist_test3 = distribution_new_custom(
      28,
      (double[]){1,          4,           6,          19.3,       65.35,
                 98.9423,    904.4321,    1000.432,   7894.90145, 8000.5472,
                 9000.852,   10942.11,    11443,      89002.432,  90423.62,
                 95326.54,   97642.90,    100432.75,  109543.62,  209536.3543,
                 500426.626, 635690.62,   790426.268, 800738.374, 1000436.637,
                 1111111.98, 1234567.890, 2345678.901});
  distribution_t *dist_test4 = distribution_new_linear(30, 1.5);

  struct {
    distribution_t *input_dist;
    int num_buckets;
  } cases[] = {{
                   .input_dist = NULL,
                   .num_buckets = 0,
               },
               {
                   .input_dist = dist_test1,
                   .num_buckets = 5,
               },
               {
                   .input_dist = dist_test2,
                   .num_buckets = 14,
               },
               {
                   .input_dist = dist_test3,
                   .num_buckets = 29,
               },
               {
                   .input_dist = dist_test4,
                   .num_buckets = 30,
               }};

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    EXPECT_EQ_UINT64(cases[i].num_buckets,
                     distribution_get_num_buckets(cases[i].input_dist));

    distribution_destroy(cases[i].input_dist);
  }

  return 0;
}

DEF_TEST(distribution_get_buckets_boundaries) {
  distribution_t *dist_test1 = distribution_new_linear(5, 15.0);
  distribution_t *dist_test2 = distribution_new_exponential(14, 1.5, 4);
  distribution_t *dist_test3 = distribution_new_custom(
      28,
      (double[]){1,          4,           6,          19.3,       65.35,
                 98.9423,    904.4321,    1000.432,   7894.90145, 8000.5472,
                 9000.852,   10942.11,    11443,      89002.432,  90423.62,
                 95326.54,   97642.90,    100432.75,  109543.62,  209536.3543,
                 500426.626, 635690.62,   790426.268, 800738.374, 1000436.637,
                 1111111.98, 1234567.890, 2345678.901});
  distribution_t *dist_test4 = distribution_new_linear(30, 1.5);

  struct {
    distribution_t *input_dist;
    int num_buckets;
    double *want_boundaries;
    int cannot_be_free;
  } cases[] = {
      {
          .input_dist = NULL,
          .want_boundaries = NULL,
          .num_buckets = 0,
      },
      {
          .input_dist = dist_test1,
          .num_buckets = 5,
          .want_boundaries = array_new_linear(5, 15.0),
      },
      {
          .input_dist = dist_test2,
          .num_buckets = 14,
          .want_boundaries = array_new_exponential(14, 1.5, 4),
      },
      {
          .input_dist = dist_test3,
          .num_buckets = 29,
          .want_boundaries =
              (double[]){
                  1,          4,           6,           19.3,       65.35,
                  98.9423,    904.4321,    1000.432,    7894.90145, 8000.5472,
                  9000.852,   10942.11,    11443,       89002.432,  90423.62,
                  95326.54,   97642.90,    100432.75,   109543.62,  209536.3543,
                  500426.626, 635690.62,   790426.268,  800738.374, 1000436.637,
                  1111111.98, 1234567.890, 2345678.901, INFINITY},
          .cannot_be_free = 1,
      },
      {
          .input_dist = dist_test4,
          .num_buckets = 30,
          .want_boundaries = array_new_linear(30, 1.5),
      }};

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    EXPECT_EQ_UINT64(cases[i].num_buckets,
                     distribution_get_num_buckets(cases[i].input_dist));

    double *boundaries =
        distribution_get_buckets_boundaries(cases[i].input_dist);

    if (cases[i].want_boundaries == NULL) {
      EXPECT_EQ_PTR(cases[i].want_boundaries, boundaries);
    } else {
      CHECK_NOT_NULL(boundaries);
      for (size_t j = 0; j < cases[i].num_buckets; ++j) {
        EXPECT_EQ_DOUBLE(cases[i].want_boundaries[j], boundaries[j]);
      }
      free(boundaries);

      if (cases[i].cannot_be_free == 0) {
        free(cases[i].want_boundaries);
      }
    }

    distribution_destroy(cases[i].input_dist);
  }

  return 0;
}

DEF_TEST(distribution_get_buckets_counters) {
  distribution_t *dist_test1 = distribution_new_linear(10, 5.0);
  distribution_t *dist_test2 = distribution_new_exponential(8, 1.5, 2);
  distribution_t *dist_test3 = distribution_new_custom(
      11, (double[]){1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144});

  struct {
    distribution_t *input_dist;
    double *gauges;
    int num_queries;
    uint64_t *counters;
    int num_buckets;
    double sum_gauges;
  } cases[] = {
      {
          .input_dist = NULL,
          .num_queries = 0,
          .num_buckets = 0,
          .sum_gauges = NAN,
      },
      {
          .input_dist = dist_test1,
          .num_queries = 11,
          .gauges = (double[]){1, 2, 3, 5, 10, 90, 8, 45, 44, 41.45, 40.5},
          .num_buckets = 10,
          .counters = (uint64_t[]){3, 5, 6, 6, 6, 6, 6, 6, 9, 11},
          .sum_gauges = 289.95,
      },
      {
          .input_dist = dist_test2,
          .num_queries = 16,
          .gauges =
              (double[]){1.5, 1.23, 1.67, 2, 24.532, 25, 28.43, 98.43, 10.43,
                         7.53, 11.235, 4.43256, 7.432, 3, 3.01, 2.98},
          .num_buckets = 8,
          .counters = (uint64_t[]){3, 5, 8, 8, 10, 12, 12, 16},
          .sum_gauges = 232.84156,
      },
      {
          .input_dist = dist_test3,
          .num_queries = 15,
          .gauges = (double[]){0, 0.65, 0.7, 0.99, 0.999999, 1, 2.65, 3, 3.1123,
                               10.923, 90.432, 145.90, 144, 143.999999, 190},
          .num_buckets = 12,
          .counters = (uint64_t[]){5, 6, 7, 9, 9, 10, 10, 10, 10, 10, 12, 15},
          .sum_gauges = 738.357298,
      }};

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    for (int j = 0; j < cases[i].num_queries; ++j) {
      distribution_update(cases[i].input_dist, cases[i].gauges[j]);
    }

    EXPECT_EQ_UINT64(cases[i].num_buckets,
                     distribution_get_num_buckets(cases[i].input_dist));

    uint64_t *counters = distribution_get_buckets_counters(cases[i].input_dist);

    if (cases[i].counters == NULL) {
      EXPECT_EQ_PTR(cases[i].counters, counters);
    } else {
      CHECK_NOT_NULL(counters);
      for (size_t j = 0; j < cases[i].num_buckets; ++j) {
        EXPECT_EQ_UINT64(cases[i].counters[j], counters[j]);
      }
      free(counters);
    }

    EXPECT_EQ_DOUBLE(cases[i].sum_gauges,
                     distribution_get_sum_gauges(cases[i].input_dist));
    distribution_destroy(cases[i].input_dist);
  }

  return 0;
}

DEF_TEST(distribution_check_equal) {
  distribution_t *dist_test11 = distribution_new_linear(15, 17.97);
  distribution_t *dist_test22 = distribution_new_exponential(19, 1.497, 7.9);
  distribution_t *dist_test31 = distribution_new_exponential(15, 2.1, 3);
  distribution_t *dist_test32 = distribution_new_exponential(15, 2.1, 3.0001);
  distribution_t *dist_test41 = distribution_new_linear(8, 21);
  distribution_t *dist_test42 = distribution_new_custom(
      7, (double[]){21, 42, 63, 84, 105, 126, 147});

  struct {
    distribution_t *input_dist1;
    distribution_t *input_dist2;
    double *gauges1;
    double *gauges2;
    int num_queries1;
    int num_queries2;
    int want_get;
  } cases[] = {
      {
          .input_dist1 = NULL,
          .input_dist2 = NULL,
          .num_queries1 = 0,
          .num_queries2 = 0,
          .want_get = 1,
      },
      {
          .input_dist1 = dist_test11,
          .input_dist2 = NULL,
          .num_queries1 = 17,
          .num_queries2 = 0,
          .gauges1 =
              (double[]){64.986822, 75.361073, 291.412027, 0.209184, 25.542358,
                         207.719335, 228.715725, 120.862435, 50.335099,
                         78.147062, 103.449701, 45.456052, 120.827738,
                         39.133311, 66.804762, 256.750525, 42.075292},
      },
      {
          .input_dist1 = NULL,
          .input_dist2 = dist_test22,
          .num_queries1 = 0,
          .num_queries2 = 21,
          .gauges2 =
              (double[]){39443.618618, 24339.664702, 21573.536089, 29609.30347,
                         2926.352621,  14058.673966, 4660.770634,  29783.728304,
                         34311.842208, 8530.487236,  19927.009242, 15188.572656,
                         971.651245,   16269.067161, 2591.089086,  22718.987438,
                         28344.842898, 17402.4872,   31390.581462, 24385.094319,
                         29730.582344},
      },
      {
          .input_dist1 = dist_test31,
          .input_dist2 = dist_test32,
          .num_queries1 = 15,
          .num_queries2 = 15,
          .gauges1 =
              (double[]){91162.43496, 72940.539939, 84641.174039, 97027.221525,
                         84159.235853, 91894.852013, 52426.443153, 27785.207936,
                         14766.938133, 94843.147406, 79763.869899, 32806.450583,
                         74097.374659, 3293.59171, 6341.594074},
          .gauges2 =
              (double[]){91162.43496, 72940.539939, 84641.174039, 97027.221525,
                         84159.235853, 91894.852013, 52426.443153, 27785.207936,
                         14766.938133, 94843.147406, 79763.869899, 32806.450583,
                         74097.374659, 3293.59171, 6341.594074},
      },
      {
          .input_dist1 = dist_test41,
          .input_dist2 = dist_test42,
          .num_queries1 = 17,
          .num_queries2 = 17,
          .gauges1 = (double[]){122.793488, 73.629423, 85.238252, 171.841943,
                                189.006106, 92.612949, 83.502165, 139.368244,
                                27.286445, 77.298995, 56.650835, 163.273312,
                                142.017526, 162.949669, 31.717699, 38.69047,
                                175.971837},
          .gauges2 = (double[]){122.793488, 73.629423, 85.238252, 171.841943,
                                189.006106, 92.612949, 83.502165, 139.368244,
                                27.286445, 77.298995, 56.650835, 163.273312,
                                142.017526, 162.949669, 31.717699, 38.69047,
                                175.971837},
          .want_get = 1,
      }};

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    for (int j = 0; j < cases[i].num_queries1; ++j) {
      distribution_update(cases[i].input_dist1, cases[i].gauges1[j]);
    }

    for (int j = 0; j < cases[i].num_queries2; ++j) {
      distribution_update(cases[i].input_dist2, cases[i].gauges2[j]);
    }

    EXPECT_EQ_INT(
        cases[i].want_get,
        distribution_check_equal(cases[i].input_dist1, cases[i].input_dist2));

    distribution_destroy(cases[i].input_dist1);
    distribution_destroy(cases[i].input_dist2);
  }

  return 0;
}

DEF_TEST(distribution_get_sum_gauges) {
  distribution_t *dist_test1 = distribution_new_linear(10, 5.0);
  distribution_t *dist_test2 = distribution_new_exponential(8, 1.5, 2);
  distribution_t *dist_test3 = distribution_new_custom(
      11, (double[]){1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144});

  struct {
    distribution_t *input_dist;
    double *gauges;
    int num_queries;
    double sum_gauges;
  } cases[] = {
      {
          .input_dist = NULL,
          .num_queries = 0,
          .sum_gauges = NAN,
      },
      {
          .input_dist = dist_test1,
          .num_queries = 19,
          .gauges = (double[]){103.022105, 171.636117, 116.488605, 28.172234,
                               36.809295, 105.699156, 95.190406, 173.762403,
                               105.859558, 105.500904, 42.080885, 145.297908,
                               109.747067, 183.684136, 27.112998, 43.693238,
                               184.177938, 138.033766, 171.255309},
          .sum_gauges = 2087.224028,
      },
      {
          .input_dist = dist_test2,
          .num_queries = 18,
          .gauges = (double[]){1314.386028, 713.040721, 388.600533, 2194.733454,
                               62.713018, 436.076538, 154.274781, 467.571249,
                               1956.153932, 1884.719494, 1744.740075,
                               715.797969, 686.73603, 223.723816, 202.431405,
                               1640.915258, 1236.700456, 1328.934664},
          .sum_gauges = 17352.249421,
      },
      {
          .input_dist = dist_test3,
          .num_queries = 21,
          .gauges = (double[]){85.440604,  89.039631,  197.239067, 20.809416,
                               23.130686,  109.073608, 236.542966, 158.416868,
                               30.537857,  155.668704, 202.337704, 127.671802,
                               33.857584,  95.56406,   63.416192,  188.410385,
                               140.583885, 96.033746,  70.10835,   110.265126,
                               95.71921},
          .sum_gauges = 2329.867451,
      }};

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    for (int j = 0; j < cases[i].num_queries; ++j) {
      distribution_update(cases[i].input_dist, cases[i].gauges[j]);
    }

    char buffer[256];
    double sum = distribution_get_sum_gauges(cases[i].input_dist);
    snprintf(buffer, 256, "%.6lf", sum);
    sscanf(buffer, "%lf", &sum);
    EXPECT_EQ_DOUBLE(cases[i].sum_gauges, sum);

    distribution_destroy(cases[i].input_dist);
  }

  return 0;
}

int main(void) {
  RUN_TEST(distribution_new_linear);
  RUN_TEST(distribution_new_exponential);
  RUN_TEST(distribution_new_custom);
  RUN_TEST(distribution_update);
  RUN_TEST(distribution_percentile);
  RUN_TEST(distribution_average);
  RUN_TEST(distribution_clone);
  RUN_TEST(distribution_get_num_buckets);
  RUN_TEST(distribution_get_buckets_boundaries);
  RUN_TEST(distribution_get_buckets_counters);
  RUN_TEST(distribution_get_sum_gauges);
  RUN_TEST(distribution_check_equal);

  END_TEST;
}