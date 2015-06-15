all:
	g++ -DASIO_STANDALONE -DBOOST_ASIO_ENABLE_HANDLER_TRACKING \
	-std=c++11 -Ilib/asio-1.11.0/include -Wall  \
	src/main.cpp src/market/market.cpp src/market/trading.cpp src/server/async_server.cpp \
	src/server/async_connection.cpp src/misc/misc.cpp -o bin/cms
