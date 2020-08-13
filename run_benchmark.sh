#!/bin/bash

for i in {1..4000..50}
do
  ./distribution_benchmark $i
done
