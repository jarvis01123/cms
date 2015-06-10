all:
	g++ -DASIO_STANDALONE -DBOOST_ASIO_ENABLE_HANDLER_TRACKING \
	-std=c++11 -I../../lib/asio-1.11.0/include -Wall  \
	main.cpp market.cpp trading.cpp ../server/async_server.cpp \
	../server/async_connection.cpp ../misc/misc.cpp -o cms
