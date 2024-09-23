#ifndef CELERIS_HPP
#define CELERIS_HPP

#include <boost/asio.hpp>
#include <functional>
#include <string>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>
#include "json.hpp"
#include "request.hpp"
#include "response.hpp"
#include "router.hpp"

// ANSI escape codes for colored output
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define RED "\033[31m"

// Helper function to print colorful debug logs
inline void print_debug(const std::string& message, const std::string& color) {
    std::cout << color << message << RESET << std::endl;
}

class Celeris : private Router {
public:
    explicit Celeris(unsigned short port, const std::string& address = "127.0.0.1")
        : io_context_(), acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port)) {}

    std::string get_reason_phrase(int status_code) {
        static const std::map<int, std::string> status_codes = {
            {200, "OK"},
            {201, "Created"},
            {202, "Accepted"},
            {204, "No Content"},
            {206, "Partial Content"},
            {301, "Moved Permanently"},
            {302, "Found"},
            {304, "Not Modified"},
            {400, "Bad Request"},
            {401, "Unauthorized"},
            {403, "Forbidden"},
            {404, "Not Found"},
            {405, "Method Not Allowed"},
            {406, "Not Acceptable"},
            {409, "Conflict"},
            {413, "Request Entity Too Large"},
            {414, "Request-URI Too Long"},
            {415, "Unsupported Media Type"},
            {416, "Requested Range Not Satisfiable"},
            {417, "Expectation Failed"},
            {418, "I'm a teapot"},
            {500, "Internal Server Error"},
            {501, "Not Implemented"},
            {502, "Bad Gateway"},
            {503, "Service Unavailable"},
            {504, "Gateway Timeout"},
            {505, "HTTP Version Not Supported"}
        };

        auto it = status_codes.find(status_code);
        if (it != status_codes.end()) {
            return it->second;
        }
        return "Unknown Status";
    }

    void get(const std::string& path, std::function<void(const Request&, Response&)> handler) {
        register_route(path, [handler](std::shared_ptr<boost::asio::ip::tcp::socket> socket, Request& req, Response& res) {
            if (req.method == "GET") {
                handler(req, res);
            } else {
                res.set_error(405, "Method Not Allowed: '" + req.method + "'");
            }
        });
    }

    void post(const std::string& path, std::function<void(const Request&, Response&)> handler) {
        register_route(path, [handler](std::shared_ptr<boost::asio::ip::tcp::socket> socket, Request& req, Response& res) {
            if (req.method == "POST") {
                handler(req, res);
            } else {
                res.set_error(405, "Method Not Allowed '" + req.method + "'");
            }
        });
    }

    void listen() {
        print_debug("Celeris server running on http://" + acceptor_.local_endpoint().address().to_string() + ":" + std::to_string(acceptor_.local_endpoint().port()) + " (Press CTRL+C to quit)", GREEN);
        start_accept();
        io_context_.run();
    }

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;

    void register_route(const std::string& path, std::function<void(std::shared_ptr<boost::asio::ip::tcp::socket>, Request&, Response&)> handler) {
        Router::register_route(path, handler);
    }

    void start_accept() {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
            if (!ec) {
                handle_request(socket);
            } else {
                print_debug("Error accepting connection: " + ec.message(), RED);
            }
            start_accept();
        });
    }

    void handle_request(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
        auto buffer = std::make_shared<boost::asio::streambuf>();

        boost::asio::async_read_until(*socket, *buffer, "\r\n\r\n",
            [this, socket, buffer](const boost::system::error_code& error, std::size_t) {
                if (!error) {
                    std::istream stream(buffer.get());
                    std::string request_line;
                    std::getline(stream, request_line);

                    std::string method, path, version;
                    std::istringstream request_stream(request_line);
                    request_stream >> method >> path >> version;

                    print_debug("Received Request: " + method + " " + path + " " + version, CYAN);

                    std::map<std::string, std::string> headers;
                    std::string header_line;
                    while (std::getline(stream, header_line) && header_line != "\r") {
                        auto colon_pos = header_line.find(':');
                        if (colon_pos != std::string::npos) {
                            std::string header_name = header_line.substr(0, colon_pos);
                            std::string header_value = header_line.substr(colon_pos + 2);
                            headers[header_name] = header_value;
                        }
                    }

                    std::string body;
                    if (headers.count("Content-Length")) {
                        std::size_t content_length = std::stoul(headers["Content-Length"]);
                        body.resize(content_length);
                        stream.read(&body[0], content_length);
                    }

                    Request req(method, path, headers, body);
                    Response res;

                    auto route_it = Router::routes_.find(path);
                    if (route_it != Router::routes_.end()) {
                        route_it->second(socket, req, res);
                    } else {
                        res.set_error(404, "Route Not Found");
                    }

                    std::string reason_phrase = get_reason_phrase(res.status_code);
                    print_debug("Response Sent: HTTP/1.1 " + std::to_string(res.status_code) + " " + reason_phrase, res.status_code >= 400 ? RED : GREEN);

                    boost::asio::async_write(*socket, boost::asio::buffer(res.to_string()),
                        [socket](const boost::system::error_code& error, std::size_t) {
                            if (!error) {
                                boost::system::error_code shutdown_ec;
                                socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, shutdown_ec);
                                if (shutdown_ec) {
                                    print_debug("Error shutting down socket: " + shutdown_ec.message(), RED);
                                }
                            } else {
                                print_debug("Error sending response: " + error.message(), RED);
                            }
                        });
                } else {
                    print_debug("Error reading request: " + error.message(), RED);
                }
            });
    }
};

#endif // CELERIS_HPP
