main : main.cpp 
	g++ -lm -s -O2 main.cpp -o main

benchmark: main
	./benchmark.sh | awk '{ sum += $$1 } END { print sum }'

benchmark_small: main
	./benchmark_small.sh

analytics: main
	./analytics.sh > analytics.csv
