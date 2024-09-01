// server.cpp
#include "server.hpp"
#include "router.hpp" // Include router to handle requests
#include <iostream>

Server::Server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    start_accept();
}

void Server::start_accept() {
    auto socket = std::make_shared<tcp::socket>(acceptor_.get_executor());
    acceptor_.async_accept(*socket,
        [this, socket](const boost::system::error_code& error) {
            if (!error) {
                // std::cout << "New connection accepted!" << std::endl;
                Router::handle_request(socket);  // Pass the socket to the router for handling
            } else {
                std::cerr << "Error accepting connection: " << error.message() << std::endl;
            }
            start_accept();  // Accept the next connection
        });
}
