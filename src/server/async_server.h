#pragma once

#include <iostream>
#include <string>
#include <asio.hpp>
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

        // spawn new connection and wait for client
        void accept();

        void start();
        
        void set_max_connections(int);
        trading::market& market();
        void notify_connect_closed();
        void notify_connect_open();

        // return shared pointer to new server
        static server_pointer create(asio::io_service& io_service, size_t port_no,
            trading::market& market);

    private:

        // private constructor, forces client to use create method
        async_server(asio::io_service& io_service,
            size_t port_no, trading::market& market);

        // check to see if connections have closed, shuts down io_service
        void stop_if_idle();

        // handler for accept
        void handle_accept(servers::pointer new_connection,
          const asio::error_code& error);

        tcp::acceptor _acceptor;
        int _num_connections;
        int _MAX_CONNECTIONS;
        std::mutex _num_connects_mutex;
        trading::market& _market;
    };
};
