TARGETS := context counter
all: $(TARGETS)
clean:
	rm $(TARGETS) 2>/dev/null
%: %.cpp
	g++ -O3 -I/home/vivek/boost_1_80_0-install/include -L/home/vivek/boost_1_80_0-install/lib -o $@ $^ -lboost_context -lpthread
