#include "celeris.hpp"
#include <iostream>

Celeris::Celeris(unsigned short port)
    : io_context_(), acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {}

void Celeris::get(const std::string& path, std::function<void(const Request&, Response&)> handler) {
    register_route(path, [handler](std::shared_ptr<boost::asio::ip::tcp::socket> socket, Request& req, Response& res) {
        if (req.method == "GET") {
            handler(req, res);
        } else {
            res.set_error(405, "Method Not Allowed: '"+ req.method + "'");
        }
    });
}

void Celeris::post(const std::string& path, std::function<void(const Request&, Response&)> handler) {
    register_route(path, [handler](std::shared_ptr<boost::asio::ip::tcp::socket> socket, Request& req, Response& res) {
        if (req.method == "POST") {
            handler(req, res);
        } else {
            res.set_error(405, "Method Not Allowed '"+ req.method + "'");
        }
    });
}

void Celeris::register_route(const std::string& path, std::function<void(std::shared_ptr<boost::asio::ip::tcp::socket>, Request&, Response&)> handler) {
    Router::register_route(path, handler);
}

void Celeris::start() {
    std::cout << "Celeris server running on port " << acceptor_.local_endpoint().port() << std::endl;
    start_accept();
    io_context_.run();
}

void Celeris::start_accept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            handle_request(socket);
        } else {
            std::cerr << "Error accepting connection: " << ec.message() << std::endl;
        }
        start_accept();
    });
}

void Celeris::handle_request(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
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

                std::map<std::string, std::string> headers;
                std::string header_line;
                while (std::getline(stream, header_line) && header_line != "\r") {
                    auto colon_pos = header_line.find(':');
                    if (colon_pos != std::string::npos) {
                        std::string header_name = header_line.substr(0, colon_pos);
                        std::string header_value = header_line.substr(colon_pos + 2); // Adjust for space after ':'
                        headers[header_name] = header_value;
                    }
                }

                // Reading the body if Content-Length is present
                std::string body;
                if (headers.count("Content-Length")) {
                    std::size_t content_length = std::stoul(headers["Content-Length"]);
                    body.resize(content_length);
                    stream.read(&body[0], content_length);
                }

                Request req(method, path, headers, body);
                Response res;

                // Route handling
                auto route_it = Router::routes_.find(path);
                if (route_it != Router::routes_.end()) {
                    route_it->second(socket, req, res);
                } else {
                    res.set_error(404, "Route Not Found");
                }

                // Sending the response
                boost::asio::async_write(*socket, boost::asio::buffer(res.to_string()),
                    [socket](const boost::system::error_code& error, std::size_t) {
                        if (!error) {
                            boost::system::error_code shutdown_ec;
                            socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, shutdown_ec);
                            if (shutdown_ec) {
                                std::cerr << "Error shutting down socket: " << shutdown_ec.message() << std::endl;
                            }
                        } else {
                            std::cerr << "Error sending response: " << error.message() << std::endl;
                        }
                    });
            } else {
                std::cerr << "Error reading request: " << error.message() << std::endl;
            }
        });
}

void Celeris::listen() {
    start();
}
