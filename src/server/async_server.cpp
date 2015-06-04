#include "async_server.h"

servers::async_server::async_server(asio::io_service& io_service,
    size_t port_no, trading::market& market)
    : _acceptor(io_service, tcp::endpoint(tcp::v4(), port_no)),
      _num_connections{0},
      _DEBUG{false},
      _market{market} { }

void servers::async_server::accept() {

    servers::pointer new_connection =
      servers::async_connection::create(_acceptor.get_executor().context(), shared_from_this());

    _acceptor.async_accept(new_connection->socket(),
        bind(&async_server::handle_accept, this, new_connection,
          std::placeholders::_1));
}

void servers::async_server::stop_if_idle() {
    if (_num_connections <= 1) {
        _acceptor.get_executor().context().stop();
    }
}

void servers::async_server::notify_connect_closed() {

    std::lock_guard<std::mutex> lk(_num_connects_mutex);

    --_num_connections;
    stop_if_idle();
}

void servers::async_server::notify_connect_open() {

    std::lock_guard<std::mutex> lk(_num_connects_mutex);
    ++_num_connections;
}

void servers::async_server::handle_accept(servers::pointer new_connection,
  const asio::error_code& error) {

    // immediately accept incoming connections
    accept();

    if (_DEBUG) {
        // put the thread to sleep to test concurrency, if debugging
        // std::this_thread::sleep_for (std::chrono::seconds(2));
    }

    if (!error) {
        new_connection->read();
    }

}

trading::market& servers::async_server::market() {
    return _market;
}

servers::server_pointer servers::async_server::create(asio::io_service& io_service,
    size_t port_no, trading::market& market) {
    return servers::server_pointer(new servers::async_server(io_service, port_no, market));
}
