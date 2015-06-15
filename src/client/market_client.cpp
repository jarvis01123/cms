#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <functional>
#include "market.h"
#include <asio.hpp>
#include <thread>
#include <chrono>



using asio::ip::tcp;


int main(int argc, char const *argv[]) {

    asio::io_service io_service;
    tcp::resolver resolver(io_service);

    tcp::resolver::query query(argv[1], argv[2],
        asio::ip::resolver_query_base::numeric_service);


    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);



    std::array<char, 256> buf;
    std::string message;
    asio::error_code ignored_error;

    tcp::socket socket(io_service);
    asio::connect(socket, endpoint_iterator);

    while( getline(std::cin, message) ) {

        if (message == "close") {
            socket.close();
            exit(0);
        }

        asio::write(socket, asio::buffer(message), ignored_error);


        asio::error_code error;
        size_t len = socket.read_some(asio::buffer(buf), error);

        std::cout << std::string(buf.data(), len) << std::endl;
    }





    return 0;
}
