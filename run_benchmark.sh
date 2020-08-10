#!/bin/bash

for i in {1..200}
do
  ./distribution_benchmark $((20 * $i))
done
