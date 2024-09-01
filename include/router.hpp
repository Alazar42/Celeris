#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <boost/asio.hpp>
#include <memory>
#include <map>
#include <functional>
#include "request.hpp"
#include "response.hpp"

using boost::asio::ip::tcp;

class Router {
public:
    using RequestHandler = std::function<void(std::shared_ptr<tcp::socket>, Request&, Response&)>;
    static void register_route(const std::string& path, RequestHandler handler);
    static void handle_request(std::shared_ptr<tcp::socket> socket);
    static std::map<std::string, RequestHandler> routes_;
};

#endif // ROUTER_HPP
