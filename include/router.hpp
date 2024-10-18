#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <boost/asio.hpp>
#include <memory>
#include <map>
#include <functional>
#include <iostream>
#include <sstream>
#include "json.hpp"
#include "request.hpp"
#include "response.hpp"

using boost::asio::ip::tcp;

class Router
{
public:
    using RequestHandler = std::function<void(std::shared_ptr<tcp::socket>, Request &, Response &)>;

    // Registers a route with its associated handler for a specific method
    void register_route(const std::string &method, const std::string &path, RequestHandler handler)
    {
        routes_[method + ":" + path] = handler; // Use method + ":" + path as key
    }

    // Convenience function for registering GET routes
    void register_get(const std::string &path, RequestHandler handler)
    {
        register_route("GET", path, handler);
    }

    // Convenience function for registering POST routes
    void register_post(const std::string &path, RequestHandler handler)
    {
        register_route("POST", path, handler);
    }

    // Handles routing based on the request method and path
    bool handle_router_request(std::shared_ptr<tcp::socket> socket, Request &req, Response &res)
    {
        std::string route_key = req.get_method() + ":" + req.get_path(); // Match the key format

        auto it = routes_.find(route_key);
        if (it != routes_.end())
        {
            // Route is found, call the associated handler
            it->second(socket, req, res);
            return true;
        }
        else
        {
            // No exact route found; check for a different method on the same path
            std::string other_method = (req.get_method() == "POST") ? "GET" : "POST";
            std::string method_check_key = other_method + ":" + req.get_path(); // Check if the other method exists

            if (routes_.find(method_check_key) != routes_.end())
            {
                res.set_error(405, "Method Not Allowed");
                return true; // Method not allowed
            }

            std::cout << format_error_message("Route not found for: " + route_key) << std::endl; // Debug log for route not found
            return false; // No route found
        }
    }

    // Handles incoming requests
    void handle_incoming_request(std::shared_ptr<tcp::socket> socket)
    {
        auto buffer = std::make_shared<boost::asio::streambuf>();

        boost::asio::async_read_until(*socket, *buffer, "\r\n\r\n",
                                      [this, socket, buffer](const boost::system::error_code &error, std::size_t)
                                      {
                                          if (!error)
                                          {
                                              std::istream stream(buffer.get());
                                              std::string request_line;
                                              std::getline(stream, request_line);

                                              std::string method, path, version;
                                              std::istringstream request_stream(request_line);
                                              request_stream >> method >> path >> version;

                                              // Log the request method and path
                                              std::cout << "Received request: " << method << " " << path << std::endl;

                                              // Parse headers and body
                                              std::map<std::string, std::string> headers;
                                              std::string header_line;
                                              while (std::getline(stream, header_line) && header_line != "\r")
                                              {
                                                  auto colon_pos = header_line.find(':');
                                                  if (colon_pos != std::string::npos)
                                                  {
                                                      std::string header_name = header_line.substr(0, colon_pos);
                                                      std::string header_value = header_line.substr(colon_pos + 1);
                                                      // Trim leading whitespace
                                                      header_value.erase(0, header_value.find_first_not_of(" \t"));
                                                      headers[header_name] = header_value;
                                                  }
                                              }

                                              // Check for Content-Length header
                                              std::string body;
                                              if (headers.find("Content-Length") != headers.end())
                                              {
                                                  std::size_t content_length = std::stoul(headers["Content-Length"]);
                                                  body.resize(content_length);
                                                  stream.read(&body[0], content_length);
                                                  if (stream.gcount() < content_length)
                                                  {
                                                      std::cerr << "Warning: Read less than Content-Length" << std::endl;
                                                  }
                                              }

                                              // Create Request and Response objects
                                              Request req(method, path, headers, body);
                                              Response res;

                                              // Parse JSON if the method is POST and Content-Type is application/json
                                              if (req.get_method() == "POST" && req.has_header("Content-Type") && req.get_header("Content-Type") == "application/json")
                                              {
                                                  try
                                                  {
                                                      auto json_data = req.parse_json();
                                                      // Optionally process json_data here if needed
                                                  }
                                                  catch (const std::runtime_error &e)
                                                  {
                                                      std::cerr << format_error_message("Error parsing JSON: " + std::string(e.what())) << std::endl;
                                                      res.set_error(400, "Invalid JSON");
                                                      // Send the error response
                                                      boost::asio::async_write(*socket, boost::asio::buffer(res.to_string()),
                                                                               [socket](const boost::system::error_code &error, std::size_t)
                                                                               {
                                                                                   if (!error)
                                                                                   {
                                                                                       // Gracefully close the connection
                                                                                       socket->shutdown(tcp::socket::shutdown_both);
                                                                                       socket->close();
                                                                                   }
                                                                                   else
                                                                                   {
                                                                                       std::cerr << "Error sending response: " << error.message() << std::endl;
                                                                                   }
                                                                               });
                                                      return; // Early return to avoid further processing
                                                  }
                                              }

                                              // Route the request
                                              bool route_found = handle_router_request(socket, req, res);

                                              if (!route_found)
                                              {
                                                  // If no route is found, send a 404 error
                                                  res.set_error(404, "Not Found");
                                              }

                                              // Send response
                                              boost::asio::async_write(*socket, boost::asio::buffer(res.to_string()),
                                                                       [socket](const boost::system::error_code &error, std::size_t)
                                                                       {
                                                                           if (!error)
                                                                           {
                                                                               // Gracefully close the connection
                                                                               socket->shutdown(tcp::socket::shutdown_both);
                                                                               socket->close();
                                                                           }
                                                                           else
                                                                           {
                                                                               std::cerr << "Error sending response: " << error.message() << std::endl;
                                                                           }
                                                                       });
                                          }
                                          else
                                          {
                                              std::cerr << "Error reading request: " << error.message() << std::endl;
                                          }
                                      });
    }

private:
    // Map to store routes with their handlers
    std::map<std::string, RequestHandler> routes_;

    // Format error message to display in red
    static std::string format_error_message(const std::string &message)
    {
        return "\033[31m" + message + "\033[0m"; // ANSI escape code for red text
    }
};

#endif // ROUTER_HPP
