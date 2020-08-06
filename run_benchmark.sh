#!/bin/bash

for i in {0..20}
do
  echo Calculating benchmark for: $((2 ** $i))
  ./distribution_benchmark $((2 ** $i))
  echo End calculating benchmark
done
