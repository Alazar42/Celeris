#ifndef CELERIS_HPP
#define CELERIS_HPP

#include "boost/asio.hpp"
#include "boost/beast/http.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/version.hpp"
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

// ANSI escape codes for colorful output
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define RED "\033[31m"

class Celeris : private Router
{
public:
    // Constructor to initialize the server with a port and optional address (defaults to localhost)
    explicit Celeris(unsigned short port, const std::string &address = "127.0.0.1")
        : io_context_(), acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port)) {}

    // Register GET route handler
    void get(const std::string &path, std::function<void(const Request &, Response &)> handler)
    {
        register_route("GET", path, [handler](std::shared_ptr<boost::asio::ip::tcp::socket> socket, Request &req, Response &res)
                       {
            if (req.method_ == "GET") {
                handler(req, res);
            } else {
                res.set_error(405, "Method Not Allowed: '" + req.method_ + "'");
            } });
    }

    // Register POST route handler
    void post(const std::string &path, std::function<void(const Request &, Response &)> handler)
    {
        register_route("POST", path, [handler](std::shared_ptr<boost::asio::ip::tcp::socket> socket, Request &req, Response &res)
                       {
            if (req.method_ == "POST") {
                handler(req, res);
            } else {
                res.set_error(405, "Method Not Allowed: '" + req.method_ + "'");
            } });
    }

    // Start listening for incoming connections
    void listen()
    {
        print_debug("Celeris server running on http://" + acceptor_.local_endpoint().address().to_string() + ":" + std::to_string(acceptor_.local_endpoint().port()) + " (Press CTRL+C to quit)", GREEN);
        start_accept();
        io_context_.run();
    }

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;

    // Register route in the router
    void register_route(const std::string &method, const std::string &path, std::function<void(std::shared_ptr<boost::asio::ip::tcp::socket>, Request &, Response &)> handler)
    {
        Router::register_route(method, path, handler);
    }

    // Start accepting incoming connections
    void start_accept()
    {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec)
                               {
            if (!ec) {
                handle_incoming_request(socket); 
            } else {
                print_debug("Error accepting connection: " + ec.message(), RED);
            }
            start_accept(); // Continue accepting new connections
        });
    }

    // Handle the incoming request
    void handle_incoming_request(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    {
        auto buffer = std::make_shared<boost::beast::flat_buffer>();
        auto req = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>();

        // Read the request
        boost::beast::http::async_read(*socket, *buffer, *req, [this, socket, buffer, req](boost::system::error_code ec, std::size_t bytes_transferred)
                                       {
            if (!ec)
            {
                // Detailed request logging
                print_debug("Received request: " + req->method_string().to_string() + " " + req->target().to_string() + " HTTP/" + std::to_string(req->version()), CYAN);

                // Create Request and Response objects
                Request custom_req(req->method_string().to_string(), req->target().to_string(), {}, req->body());
                Response custom_res;

                // Route the request using Router's method
                bool route_found = handle_router_request(socket, custom_req, custom_res); 

                if (!route_found)
                {
                    // If no route is found, send a 404 error
                    custom_res.set_error(404, "Not Found");
                }

                // Send the response
                boost::asio::async_write(*socket, boost::asio::buffer(custom_res.to_string()),
                                         [socket, req, &custom_res](const boost::system::error_code &error, std::size_t)
                                         {
                    if (!error)
                    {
                        // Gracefully close the connection
                        socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                        socket->close();
                    }
                    else
                    {
                        print_debug("Error sending response: " + error.message(), RED);
                    }
                });
            }
            else
            {
                print_debug("Error reading request: " + ec.message(), RED);
            } 
        });
    }
};

#endif // CELERIS_HPP
