mago: main.cpp algo.hpp gomoku.hpp gomoku_config.hpp util.hpp
	g++ -O3 -std=c++14 -DNDEBUG -o mago main.cpp

mago_dbg: main.cpp algo.hpp gomoku.hpp gomoku_config.hpp util.hpp
	g++ -g -O0 -std=c++14 -o mago_dbg main.cpp
