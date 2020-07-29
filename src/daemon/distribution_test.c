#include "collectd.h"

#include "distribution.h"
#include "testing.h"

DEF_TEST(distribution_new_linear) {
  struct {
    size_t num_buckets;
    double size;
    int want_err;
    double *want_get;
  } cases[] = {
      {
          .num_buckets = 0,
          .want_get = NULL,
      }
  };

  printf("##Test case: 0\n");
  distribution_t *d = distribution_new_linear(cases[0].num_buckets, cases[0].size);
  EXPECT_EQ_PTR(NULL, d);
  distribution_destroy(d);
  EXPECT_EQ_PTR(NULL, d);

  return 0;
}

int main(void) {
  RUN_TEST(distribution_new_linear);

  END_TEST;
}