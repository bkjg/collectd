#include "collectd.h"

#include "distribution.h"
#include "distribution.c"
#include "testing.h"

/* TODO(bkjg): add checking the size of returned array if is it equal to the proper size */

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
      {
        .num_buckets = 8,
        .size = 0,
        .want_get = NULL
      },
      {
        .num_buckets = 10,
        .size = 2,
        .want_get = (double[]){2, 4, 6, 8, 10, 12, 14, 16, 18, INFINITY},
      }
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    distribution_t *d = distribution_new_linear(cases[i].num_buckets, cases[i].size);

    if (cases[i].want_get == NULL) {
      EXPECT_EQ_PTR(cases[i].want_get, d);
    } else {
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
      {
        .num_buckets = 8,
        .initial_size = 0,
        .want_get = NULL
      },
      {
        .num_buckets = 12,
        .initial_size = 2,
        .factor = 0,
        .want_get = NULL
      },
      {
        .num_buckets = 33,
        .initial_size = 7,
        .factor = -5,
        .want_get = NULL
      },
      {
        .num_buckets = 6,
        .initial_size = 2,
        .factor = 3,
        .want_get = (double[]){6, 12, 24, 48, 96, INFINITY},
      }
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    distribution_t *d = distribution_new_exponential(cases[i].num_buckets, cases[i].initial_size, cases[i].factor);

    if (cases[i].want_get == NULL) {
      EXPECT_EQ_PTR(cases[i].want_get, d);
    } else {
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
      /*{
        .num_boundaries = 10,
        .want_get = NULL,
      },
      {
        .num_boundaries = 8,
        .want_get = NULL
      },
      {
        .num_boundaries = 12,
        .want_get = NULL
      },
      {
        .num_boundaries = 33,
        .want_get = NULL
      },
      {
        .num_boundaries = 6,
        .want_get = (double[]){6, 12, 24, 48, 96, INFINITY},
      }*/
  };

  for (size_t i = 0; i < (sizeof(cases) / sizeof(cases[0])); ++i) {
    printf("## Case %zu:\n", i);

    distribution_t *d = distribution_new_custom(cases[i].num_boundaries, cases[i].given_boundaries);

    if (cases[i].want_get == NULL) {
      EXPECT_EQ_PTR(cases[i].want_get, d);
    } else {
      EXPECT_EQ_UINT64(cases[i].num_boundaries + 1, d->num_buckets);

      for (size_t j = 0; j < cases[i].num_boundaries + 1; ++j) {
        EXPECT_EQ_DOUBLE(cases[i].want_get[j], d->buckets[j].max_boundary);
      }
    }

    distribution_destroy(d);
  }
  return 0;
}

int main(void) {
  RUN_TEST(distribution_new_linear);
  RUN_TEST(distribution_new_exponential);
  RUN_TEST(distribution_new_custom);

  END_TEST;
}