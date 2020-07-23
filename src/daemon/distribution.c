#include "plugin.h"
#include "distribution.h"

typedef struct {
  double max_boundary;
  uint64_t counter;
} bucket_t;

struct distribution_s {
  bucket_t *buckets;
  size_t num_buckets;
};

distribution_t *distribution_new_linear(size_t num_buckets, double size) {
  return NULL;
}

distribution_t *distribution_new_exponential(size_t num_buckets, double initial_size, double factor) {
  return NULL;
}

distribution_t *distribution_new_custom(size_t num_buckets, double *custom_buckets_sizes) {
  return NULL;
}

void distribution_update(distribution_t *d, double gauge) {

}

double distribution_percentile(distribution_t *d, double percent) {
  return 0;
}

double distribution_average(distribution_t *d) {
  return 0;
}

distribution_t distribution_clone(distribution_t *d) {
  return (distribution_t){0, 0}
}

void distribution_destroy(distribution_t *d) {

}
