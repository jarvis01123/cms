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

class market_server {


public:
    market_server(asio::io_service& io_service, int port_no)
        : _acceptor(io_service, tcp::endpoint(tcp::v4(), port_no)),
          _io_service(&io_service){

        accept();

      }

private:


    trading::market _market;
    tcp::acceptor _acceptor;
    asio::io_service * _io_service;

    void accept() {
        for (;;) {
            tcp::socket socket(*_io_service);
            _acceptor.accept(socket);


            array<char, 256> buf;
            asio::error_code error;

            size_t len = socket.read_some(asio::buffer(buf), error);

            asio::error_code ignored_error;
            std::string response = _market.str_exec(string(buf.data(), len));
            asio::write(socket, asio::buffer(response), ignored_error);


        }

    }

    // void start_accept() {
    //
    //     tcp_connection::pointer new_connection =
    //       tcp_connection::create(_acceptor.get_executor().context());
    //
    //     _acceptor.async_accept(new_connection->socket(),
    //         bind(&market_server::handle_accept, this, new_connection,
    //           asio::placeholders::error));
    // }
    //
    // void handle_accept(market_connection::pointer new_connection,
    //   const asio::error_code& error) {
    //
    //     if (!error) {
    //       new_connection->start();
    //     }
    //
    //     start_accept();
    // }

};

int main(int argc, char const *argv[]) {

    trading::market market;
    trading::order test;
    std::string message;
    std::string output;


    asio::io_service io_service;
    int port_no = 2000;

    market_server ms(io_service, port_no);



    // while (getline(std::cin, message)) {
    //
    //     if (message.length() > my::MAX) {
    //         std::cout << "Message too long" << std::endl;
    //         exit(1);
    //     }
    //
    //     output = market.str_exec(message);
    //     std::cout << output << std::endl;
    // }

    return 0;
}
