all: clean ordering noordering sequential

ordering: ordering.cpp
	g++ -o ordering -O3 ordering.cpp -lpthread
	g++ -DUSE_CPU_FENCE -o noordering -O3 ordering.cpp -lpthread
	g++ -DUSE_SINGLE_HW_THREAD -o sequential -O3 ordering.cpp -lpthread
clean:
	rm -f ordering noordering sequential 2>/dev/null
