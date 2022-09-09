all: fiber context
clean:
	rm fiber context 2>/dev/null
fiber:
	g++ -O3 -I/home/vivek/boost_1_80_0-install/include -L/home/vivek/boost_1_80_0-install/lib -o fiber fiber.cpp -lboost_fiber -lboost_context -lpthread
context: 
	g++ -O3 -I/home/vivek/boost_1_80_0-install/include -L/home/vivek/boost_1_80_0-install/lib -o context context.cpp -lboost_context
