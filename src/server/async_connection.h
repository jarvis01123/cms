#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <functional>
#include <asio.hpp>
#include <thread>
#include <chrono>
#include <future>

using asio::ip::tcp;

namespace servers {

    class async_connection;
    class async_server;

    typedef std::shared_ptr<async_connection> pointer;

    typedef std::shared_ptr<async_server> server_pointer;

    class async_connection : public std::enable_shared_from_this<async_connection> {

    public:
        static servers::async_server async_server;

        tcp::socket& socket();

        static pointer create(asio::io_service& io_service,
            servers::server_pointer server_pointer);

        void respond(std::string response);
        void read();

        // notify the server that this connection has closed
        ~async_connection();

    private:

        void handle_write(const asio::error_code&, size_t len);
        void handle_read(const asio::error_code&, size_t len );
        std::string response(std::string);
        // private constructor, force user to utilize create method
        async_connection(asio::io_service& io_service, servers::server_pointer async_server);

        tcp::socket _socket;
        std::array<char, 256> _buffer;

        // pointer to the server that spawned the connection
        server_pointer _async_server;
    };

};
