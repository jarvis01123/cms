#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <functional>
#include "market.h"
#include <asio.hpp>

using asio::ip::tcp;


int main(int argc, char const *argv[]) {

    asio::io_service io_service;
    tcp::resolver resolver(io_service);

    // tcp::resolver::query query(argv[1], 2000);
    tcp::resolver::query query(argv[1], "2000",
        asio::ip::resolver_query_base::numeric_service);


    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);



    array<char, 128> buf;
    std::string message;
    asio::error_code ignored_error;

    while( getline(std::cin, message) ) {
        tcp::socket socket(io_service);
        asio::connect(socket, endpoint_iterator);
        
        asio::write(socket, asio::buffer(message), ignored_error);


        asio::error_code error;

        size_t len = socket.read_some(asio::buffer(buf), error);

        cout << string(buf.data(), len) << endl;;
    }





    return 0;
}
