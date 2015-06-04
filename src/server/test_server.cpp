#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <functional>
#include "server.h"
#include <asio.hpp>
#include <thread>
#include <chrono>
#include <future>
#include "async_server.h"
#include "../market/market.h"

//#define TEST_SINGLE

using asio::ip::tcp;

inline void test_single_connection() {
    asio::io_service io_service;
    servers::connection connect(io_service, 2000);

    connect.accept();

    std::string message = connect.receive();

    connect.respond("Hi buddy");
}


int main(int argc, char const *argv[]) {

#ifdef TEST_SINGLE

    test_single_connection();

#endif

    asio::io_service io_service;

    trading::market market;
    servers::server_pointer market_server = servers::async_server::create(io_service, 2000, market);


    market_server->accept();

    std::vector<std::thread> threadPool;

    for(size_t t = 0; t < std::thread::hardware_concurrency(); t++){
        threadPool.push_back(std::thread([&] {

            io_service.run();  } ));
    }

    io_service.run();

    io_service.stop();
    for(std::thread& t : threadPool) {
        t.join();
    }

    return 0;
}
