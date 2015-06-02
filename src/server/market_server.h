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


class market_server {

public:

    market_server(asio::io_service& io_service, int port_no)
        : _acceptor(io_service, tcp::endpoint(tcp::v4(), port_no))
          /*_num_connects{0}*/

      }

      void initiate() {
          start_accept();
      }

private:

    class tcp_connection
        : public enable_shared_from_this<tcp_connection> {


    public:
        typedef shared_ptr<tcp_connection> pointer;

        static pointer create(asio::io_service& io_service) {
            cout << "connection created, num = " << market_server::_num_connects << endl;
            return pointer(new tcp_connection(io_service));
        }

        tcp::socket& socket() { return socket_; }

        void start() {
            message_ = "hi friend";

            cout << message_ << endl;

            asio::async_write(socket_, asio::buffer(message_),
                bind(&tcp_connection::handle_write, shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
            cout << "async write called" << endl;
        }

        ~tcp_connection() {

        }

    private:

        void handle_write(const asio::error_code& /*error*/,
            size_t /*bytes_transferred*/) {

            cout << "writing happened, num_connects = " << market_server::_num_connects << endl;
            std::this_thread::sleep_for (std::chrono::seconds(2));
            market_server::_num_connects--;

            cout << "and now, for sanity , num_connects = " << market_server::_num_connects << endl;

        }

        tcp_connection(asio::io_service& io_service)
            : socket_(io_service) { }

        tcp::socket socket_;
        std::string message_;
    };

    trading::market _market;
    tcp::acceptor _acceptor;
    static int _num_connects;

    void accept() {

        // tcp::socket socket(*_io_service);
        // _acceptor.accept(socket);
        //
        //
        // array<char, 256> buf;
        // asio::error_code error;
        //
        // size_t len = socket.read_some(asio::buffer(buf), error);
        //
        // asio::error_code ignored_error;
        // std::string response = _market.str_exec(string(buf.data(), len));
        //
        // asio::write(socket, asio::buffer(response), ignored_error);

    }

    void start_accept() {

        tcp_connection::pointer new_connection =
          tcp_connection::create(_acceptor.get_executor().context());

        _acceptor.async_accept(new_connection->socket(),
            bind(&market_server::handle_accept, this, new_connection,
              std::placeholders::_1));
    }

    void handle_accept(tcp_connection::pointer new_connection,
      const asio::error_code& error) {



        if (!error) {
            _num_connects++;
            new_connection->start();
        }


        start_accept();


        cout << "num_connects = " << _num_connects << endl;

    }

};

int market_server::_num_connects;
