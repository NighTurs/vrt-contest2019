#!/bin/sh

mkdir -p tests

for i in `seq 100`; do
      python3 generate_test.py --seed $i > tests/$i.in
done