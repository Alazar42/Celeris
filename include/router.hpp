#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <boost/asio.hpp>
#include <memory>
#include <map>
#include <functional>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "request.hpp"
#include "response.hpp"

using boost::asio::ip::tcp;

class Router {
public:
    using RequestHandler = std::function<void(std::shared_ptr<tcp::socket>, Request&, Response&)>;
    
    // Registers a route with its associated handler
    static void register_route(const std::string& path, RequestHandler handler) {
        routes_[path] = handler;
    }
    
    // Handles incoming requests
    static void handle_request(std::shared_ptr<tcp::socket> socket) {
        auto buffer = std::make_shared<boost::asio::streambuf>();

        boost::asio::async_read_until(*socket, *buffer, "\r\n\r\n",
            [socket, buffer](const boost::system::error_code& error, std::size_t) {
                if (!error) {
                    std::istream stream(buffer.get());
                    std::string request_line;
                    std::getline(stream, request_line);

                    std::string method, path, version;
                    std::istringstream request_stream(request_line);
                    request_stream >> method >> path >> version;

                    // Parse headers and body
                    std::map<std::string, std::string> headers;
                    std::string header_line;
                    while (std::getline(stream, header_line) && header_line != "\r") {
                        auto colon_pos = header_line.find(':');
                        if (colon_pos != std::string::npos) {
                            std::string header_name = header_line.substr(0, colon_pos);
                            std::string header_value = header_line.substr(colon_pos + 1);
                            headers[header_name] = header_value;
                        }
                    }

                    std::string body;
                    if (headers.find("Content-Length") != headers.end()) {
                        std::size_t content_length = std::stoul(headers["Content-Length"]);
                        body.resize(content_length);
                        stream.read(&body[0], content_length);
                    }

                    // Create Request and Response objects
                    Request req(method, path, headers, body);
                    Response res;

                    // Find and execute the route handler
                    auto it = routes_.find(path);
                    if (it != routes_.end()) {
                        it->second(socket, req, res);  // Pass request and response objects to the handler
                    } else {
                        res.status_code = 404;
                        res.body = nlohmann::json{{"detail", "404 Not Found"}, {"status", 404}}.dump();
                    }

                    // Send response
                    boost::asio::async_write(*socket, boost::asio::buffer(res.to_string()),
                        [socket](const boost::system::error_code& error, std::size_t) {
                            if (!error) {
                                socket->shutdown(tcp::socket::shutdown_both);
                                socket->close();
                            } else {
                                std::cerr << "Error sending response: " << error.message() << std::endl;
                            }
                        });
                } else {
                    std::cerr << "Error reading request: " << error.message() << std::endl;
                }
            });
    }

    // Map to store routes and their handlers
    static std::map<std::string, RequestHandler> routes_;
};

// Define the static member
std::map<std::string, Router::RequestHandler> Router::routes_;

#endif // ROUTER_HPP
