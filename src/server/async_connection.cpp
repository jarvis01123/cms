#include "async_connection.h"
#include "async_server.h"

tcp::socket& servers::async_connection::socket() { return _socket; }

servers::pointer servers::async_connection::create(asio::io_service& io_service,
                 servers::server_pointer server_pointer) {

    return servers::pointer(new servers::async_connection(io_service, server_pointer));
}

std::string servers::async_connection::response(std::string call) {
    return _async_server->market().execute(call);
}

void servers::async_connection::respond(std::string res) {
        asio::async_write(_socket, asio::buffer(res),
            bind(&servers::async_connection::handle_write, shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
}

void servers::async_connection::read() {
    _socket.async_read_some(asio::buffer(_buffer),
        bind(&servers::async_connection::handle_read, shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
}

void servers::async_connection::handle_write(const asio::error_code& /*error*/,
    size_t len /*bytes_transferred*/) { }

void servers::async_connection::handle_read(const asio::error_code& error,
    size_t len /*bytes_transferred*/) {

    // test will fail when connection closed by client
    if (error != asio::error::eof) {

        // generate response to the message from client
        auto res = response(std::string(_buffer.data(), len));
        respond(res);

        // read next instruction
        read();
    }
}

servers::async_connection::async_connection(asio::io_service& io_service,
                                            servers::server_pointer async_server)
    : _socket(io_service),
      _async_server(async_server) {

    // notify parent server that connection has been created
    _async_server->notify_connect_open();
}

servers::async_connection::~async_connection() {

    // notify parent server that connection has been destroyed
    _async_server->notify_connect_closed();
}
