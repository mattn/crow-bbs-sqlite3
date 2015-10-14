all : bbs

bbs : bbs.cxx
	g++ -std=c++14 -o bbs -I. bbs.cxx -lsqlite3 -lboost_system -lpthread
