#ifndef CELERIS_HPP
#define CELERIS_HPP

#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <string>
#include <map>
#include "request.hpp"
#include "response.hpp"

using boost::asio::ip::tcp;

class Celeris {
public:
    Celeris(boost::asio::io_context& io_context, unsigned short port);

    void start(); // Starts the server
    void register_route(const std::string& path, std::function<void(std::shared_ptr<tcp::socket>, Request&, Response&)> handler);

private:
    void start_accept(); // Declare the start_accept function here
    void handle_request(std::shared_ptr<tcp::socket> socket);

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

#endif // CELERIS_HPP
