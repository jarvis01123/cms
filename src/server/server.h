#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <functional>
#include "../market/market.h"
#include <asio.hpp>
#include <thread>
#include <chrono>
#include <future>


using asio::ip::tcp;

namespace servers {

    const size_t MAX_BUF = 256;

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

            return string(_buffer.data(), len);
        }

        void respond(std::string response) {

            asio::error_code ignored_error;
            asio::write(_socket, asio::buffer(response), ignored_error);
        }

    private:

        tcp::socket _socket;
        tcp::acceptor _acceptor;

        array<char, servers::MAX_BUF> _buffer;

    };

    class async_connection : public enable_shared_from_this<async_connection> {

    public:
        typedef shared_ptr<async_connection> pointer;

        tcp::socket& socket() { return _socket; }

        static pointer create(asio::io_service& io_service) {
            return pointer(new async_connection(io_service));
        }

        void respond(std::string response) {

            asio::async_write(_socket, asio::buffer(response),
                bind(&async_connection::handle_write, shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
        }


    private:

        void handle_write(const asio::error_code& /*error*/,
            size_t /*bytes_transferred*/) { }

        async_connection(asio::io_service& io_service)
            : _socket(io_service) { }

        tcp::socket _socket;
    };

    class async_server {

    public:

        async_server(asio::io_service& io_service, size_t port_no)
            : _acceptor(io_service, tcp::endpoint(tcp::v4(), port_no)) { }

        void accept() {

            async_connection::pointer new_connection =
              async_connection::create(_acceptor.get_executor().context());

            _acceptor.async_accept(new_connection->socket(),
                bind(&async_server::handle_accept, this, new_connection,
                  std::placeholders::_1));
        }

    private:

        void handle_accept(async_connection::pointer new_connection,
          const asio::error_code& error) {

            stringstream ss("Hello there");

            ss << " " << std::this_thread::get_id();

            cout << std::this_thread::get_id() << endl;
            
            if (!error) {
                new_connection->respond(ss.str());
            }

            accept();
        }

        tcp::acceptor _acceptor;
    };

};
