#!/bin/sh

for i in `seq 100`; do
    ./main 1 tests/$i.in
done