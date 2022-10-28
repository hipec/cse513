TARGETS := vecadd vecadd_verbose

all: clean $(TARGETS)
clean:
	rm -f $(TARGETS) 2>/dev/null
%: %.cpp
	g++ -I../compute/include -O3 -o $@ $^ -lOpenCL

