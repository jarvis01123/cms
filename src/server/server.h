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



    class connection {

    public:

        connection(asio::io_service & io_service, int port_no)
            : _socket(io_service),
            _acceptor(io_service, tcp::endpoint(tcp::v4(), port_no)) { }

        void accept() {
            _acceptor.accept(_socket);
        }

        std::string receive() {
            asio::error_code error;

            size_t len = _socket.read_some(asio::buffer(_buffer), error);

            return std::string(_buffer.data(), len);
        }

        void respond(std::string response) {

            asio::error_code ignored_error;
            asio::write(_socket, asio::buffer(response), ignored_error);
        }

    private:

        tcp::socket _socket;
        tcp::acceptor _acceptor;

        std::array<char, 256> _buffer;

    };

};
