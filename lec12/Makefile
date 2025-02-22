BOOST_INSTALL_PATH=/home/vivek/boost_1_80_0-install
TARGETS := future yield pcmutex
all: $(TARGETS)
clean:
	rm $(TARGETS) 2>/dev/null
%: %.cpp
	g++ -O3 -I$(BOOST_INSTALL_PATH)/include -L$(BOOST_INSTALL_PATH)/lib -o $@ $^ -lboost_fiber -lboost_context -lpthread
