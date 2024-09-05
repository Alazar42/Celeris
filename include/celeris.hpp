#ifndef CELERIS_HPP
#define CELERIS_HPP

#include <boost/asio.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>
#include "request.hpp"
#include "response.hpp"
#include "router.hpp"

class Celeris {
public:
    Celeris(unsigned short port);

    void get(const std::string& path, std::function<void(const Request&, Response&)> handler);
    void post(const std::string& path, std::function<void(const Request&, Response&)> handler);
    void listen();

private:
    void start();
    void start_accept();
    void handle_request(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    // Declare register_route
    void register_route(const std::string& path, std::function<void(std::shared_ptr<boost::asio::ip::tcp::socket>, Request&, Response&)> handler);

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

#endif // CELERIS_HPP
