#!/bin/sh

for i in `seq 10000`; do
    python3 generate_test.py --seed $i > tests_stress/$i.in
    ./main 0 tests_stress/$i.in > tests_stress/$i.out
    echo $i
    python3 check_solution.py --input tests_stress/$i.in --output tests_stress/$i.out
done
