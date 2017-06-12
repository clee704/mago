gomoku: gomoku.cpp algo.hpp gomoku.hpp gomoku_config.hpp util.hpp
	g++ -O3 -DNDEBUG -std=c++14 -Wall -o $@ $<

gomoku_dbg: gomoku.cpp algo.hpp gomoku.hpp gomoku_config.hpp util.hpp
	g++ -O0 -g -std=c++14 -Wall -o $@ $<
