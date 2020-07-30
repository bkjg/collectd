#include "collectd.h"

#include "distribution.c"
#include "distribution.h"
#include "testing.h"

/* TODO(bkjg): add checking the size of returned array if is it equal to the
 * proper size */
/* TODO(bkjg): add checking errno */
/* TODO(bkjg): add checking sum_gauges and counters in buckets */

DEF_TEST(distribution_new_linear) {
  struct {
    size_t num_buckets;
    double size;
    double *want_get;
  } cases[] = {
      {
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
          .want_get = (double[]){2, 4, 6, 8, 10, 12, 14, 16, 18, INFINITY},
      }};

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    distribution_t *d =
        distribution_new_linear(cases[i].num_buckets, cases[i].size);

    if (cases[i].want_get == NULL) {
      EXPECT_EQ_PTR(cases[i].want_get, d);
    } else {
      CHECK_NOT_NULL(d);
      EXPECT_EQ_UINT64(cases[i].num_buckets, d->num_buckets);

      for (size_t j = 0; j < cases[i].num_buckets; ++j) {
        EXPECT_EQ_DOUBLE(cases[i].want_get[j], d->buckets[j].max_boundary);
      }
    }

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
          .want_get = (double[]){6, 12, 24, 48, 96, INFINITY},
      }};

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    distribution_t *d = distribution_new_exponential(
        cases[i].num_buckets, cases[i].initial_size, cases[i].factor);

    if (cases[i].want_get == NULL) {
      EXPECT_EQ_PTR(cases[i].want_get, d);
    } else {
      CHECK_NOT_NULL(d);
      EXPECT_EQ_UINT64(cases[i].num_buckets, d->num_buckets);

      for (size_t j = 0; j < cases[i].num_buckets; ++j) {
        EXPECT_EQ_DOUBLE(cases[i].want_get[j], d->buckets[j].max_boundary);
      }
    }

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
      EXPECT_EQ_UINT64(cases[i].num_boundaries + 1, d->num_buckets);

      for (size_t j = 0; j < cases[i].num_boundaries + 1; ++j) {
        EXPECT_EQ_DOUBLE(cases[i].want_get[j], d->buckets[j].max_boundary);
      }
    }

    distribution_destroy(d);
  }

  return 0;
}

DEF_TEST(distribution_clone) {
  struct {
    distribution_t *input_dist;
    distribution_t *want_get;
  } cases[] = {
      {
          .input_dist = NULL,
          .want_get = NULL,
      },
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    distribution_t *d = distribution_clone(cases[i].input_dist);

    if (cases[i].want_get == NULL) {
      EXPECT_EQ_PTR(cases[i].want_get, d);
    } else {
      CHECK_NOT_NULL(d);
      // EXPECT_EQ_UINT64(cases[i].num_boundaries + 1, d->num_buckets);

      // for (size_t j = 0; j < cases[i].num_boundaries + 1; ++j) {
      //  EXPECT_EQ_DOUBLE(cases[i].want_get[j], d->buckets[j].max_boundary);
      //}
    }
  }

  return 0;
}

int main(void) {
  RUN_TEST(distribution_new_linear);
  RUN_TEST(distribution_new_exponential);
  RUN_TEST(distribution_new_custom);
  RUN_TEST(distribution_clone);

  END_TEST;
}