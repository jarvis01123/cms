all:
	g++ -DASIO_STANDALONE -DBOOST_ASIO_ENABLE_HANDLER_TRACKING -std=c++11 \
	-I../../lib/asio-1.11.0/include  -Wall \
	async_server.cpp async_connection.cpp test_server.cpp server.cpp \
	../market/market.cpp ../market/trading.cpp ../misc/misc.cpp -o serve
