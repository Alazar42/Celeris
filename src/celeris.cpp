#include "celeris.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include "request.hpp"
#include "router.hpp"
#include "response.hpp"

Celeris::Celeris(boost::asio::io_context& io_context, unsigned short port)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {}

void Celeris::start() {
    std::cout << "Celeris server running on port " << acceptor_.local_endpoint().port() << std::endl;
    start_accept();  // Begin accepting connections
    io_context_.run();
}

void Celeris::start_accept() {
    auto socket = std::make_shared<tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            handle_request(socket);
        } else {
            std::cerr << "Error accepting connection: " << ec.message() << std::endl;
        }
        start_accept();  // Continue accepting connections
    });
}

void Celeris::register_route(const std::string& path, std::function<void(std::shared_ptr<tcp::socket>, Request&, Response&)> handler) {
    Router::register_route(path, handler);
}

void Celeris::handle_request(std::shared_ptr<tcp::socket> socket) {
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
                    res.status_code = 404;
                    nlohmann::json json_response = {
                        {"detail", "Route Not Found"},
                        {"status", 404}
                    };
                    res.body = json_response.dump();
                }

                // Sending the response
                boost::asio::async_write(*socket, boost::asio::buffer(res.to_string()), 
                    [socket](const boost::system::error_code& error, std::size_t) {
                        if (!error) {
                            boost::system::error_code shutdown_ec;
                            socket->shutdown(tcp::socket::shutdown_both, shutdown_ec);
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
