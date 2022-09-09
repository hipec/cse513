EXE=fib-thread fib-async fib-deferred
all: clean $(EXE)
fib-thread:
	g++ -std=c++11 -O3 -o fib-thread fib-thread.cpp -lpthread

fib-async:
	g++ -DASYNC -std=c++11 -O3 -o fib-async fib-async.cpp -lpthread

fib-deferred:
	g++ -std=c++11 -O3 -o fib-deferred fib-async.cpp -lpthread

clean:
	rm -f $(EXE)
