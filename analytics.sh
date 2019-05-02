#!/bin/sh

for i in `seq 100`; do
    ./main 2 tests/$i.in
done
