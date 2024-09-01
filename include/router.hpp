#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <boost/asio.hpp>
#include <memory>
#include <nlohmann/json.hpp>  // Include the nlohmann JSON library

using boost::asio::ip::tcp;
using json = nlohmann::json;  // Alias for convenience

class Router {
public:
    static void handle_request(std::shared_ptr<tcp::socket> socket);
};

#endif // ROUTER_HPP
