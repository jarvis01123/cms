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

    servers::async_server async_server(io_service, 2000);

    async_server.accept();

    std::vector<std::thread> threadPool;

    for(size_t t = 0; t < std::thread::hardware_concurrency(); t++){
        threadPool.push_back(thread([&] {
            cout << "thread " << std::this_thread::get_id() << " starting" << endl;;

            io_service.run();  } ));
    }

    io_service.run();

    io_service.stop();
    for(std::thread& t : threadPool) {
        t.join();
    }

    return 0;
}
