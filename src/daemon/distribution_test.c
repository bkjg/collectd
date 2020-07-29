#include "collectd.h"

#include "distribution.h"
#include "distribution.c"
#include "testing.h"

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

int main(void) {
  RUN_TEST(distribution_new_linear);

  END_TEST;
}