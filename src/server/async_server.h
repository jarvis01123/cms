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
#include "../market/market.h"
#include "async_connection.h"

using asio::ip::tcp;

namespace servers {

    const size_t MAX_BUF = 256;

    class async_connection;
    class async_server;

    typedef std::shared_ptr<async_connection> pointer;

    typedef std::shared_ptr<async_server> server_pointer;

    class async_server : public std::enable_shared_from_this<async_server> {

    public:

        void accept();

        trading::market& market();
        void notify_connect_closed();
        void notify_connect_open();

        static server_pointer create(asio::io_service& io_service, size_t port_no,
            trading::market& market);


    private:

        async_server(asio::io_service& io_service,
            size_t port_no, trading::market& market);

        void stop_if_idle();
        void handle_accept(servers::pointer new_connection,
          const asio::error_code& error);

        tcp::acceptor _acceptor;
        int _num_connections;
        std::mutex _num_connects_mutex;
        trading::market& _market;
        bool _DEBUG;
    };
};
