#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <boost/asio.hpp>
#include <memory>

using boost::asio::ip::tcp;

class Router {
public:
    static void handle_request(std::shared_ptr<tcp::socket> socket);
};

#endif // ROUTER_HPP
