#ifndef CELERIS_HPP
#define CELERIS_HPP

#include <boost/asio.hpp>
#include <functional>
#include <string>
#include <map>
#include <memory>
#include "request.hpp"
#include "response.hpp"
#include "router.hpp"
#include "json.hpp"

class Celeris : private Router {
public:
    explicit Celeris(unsigned short port, const std::string& address = "127.0.0.1");

    std::string get_reason_phrase(int status_code);
    // Register GET and POST routes
    void get(const std::string& path, std::function<void(const Request&, Response&)> handler);
    void post(const std::string& path, std::function<void(const Request&, Response&)> handler);
    // Start the server
    void listen();

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;

    // Internal methods
    void register_route(const std::string& path, std::function<void(std::shared_ptr<boost::asio::ip::tcp::socket>, Request&, Response&)> handler);
    void start();
    void start_accept();
    void handle_request(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
};

#endif // CELERIS_HPP
