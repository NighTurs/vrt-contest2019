main : main.cpp 
	g++ -lm -s -O2 main.cpp -o main

benchmark: main
	./benchmark.sh | awk '{ sum += $$1 } END { print sum }'

analytics: main
	./analytics.sh > analytics.csv
