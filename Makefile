mago: main.cpp
	g++ -O3 -std=c++11 -DNDEBUG -o mago main.cpp

mago_dbg: main.cpp
	g++ -g -O0 -std=c++11 -o mago_dbg main.cpp
