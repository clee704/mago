CXXFLAGS = -std=c++11 -Wall
CXXFLAGS_DBG = -O0 -g
CXXFLAGS_OPT = -O3 -DNDEBUG

bin/gomoku: gomoku.cpp player.hpp gomoku.hpp gomoku_config.hpp util.hpp bin
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_OPT) -o $@ $<

bin/gomoku_dbg: gomoku.cpp player.hpp gomoku.hpp gomoku_config.hpp util.hpp bin
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_DBG) -o $@ $<

bin/othello: othello.cpp player.hpp othello.hpp util.hpp bin
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_OPT) -o $@ $<

bin/othello_dbg: othello.cpp player.hpp othello.hpp util.hpp bin
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_DBG) -o $@ $<

bin/test_util: test_util.cpp util.hpp bin
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_DBG) -o $@ $<

bin:
	mkdir bin

clean:
	rm -rf bin

.PHONY: clean
