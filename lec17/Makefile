TARGETS := matmul

all: clean $(TARGETS)
clean:
	rm -f $(TARGETS) 2>/dev/null
%: %.cpp
	g++ -I../lec16/compute/include -O3 -o $@ $^ -lOpenCL -lpthread

