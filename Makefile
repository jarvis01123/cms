all:
	g++ -DASIO_STANDALONE -std=c++11 -Iasio-1.11.0/include -Wall main.cpp market.cpp -o cms
