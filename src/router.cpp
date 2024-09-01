#include "router.hpp"
#include <iostream>
#include <string>

void Router::handle_request(std::shared_ptr<tcp::socket> socket) {
    auto buffer = std::make_shared<boost::asio::streambuf>();

    // Read until end of the HTTP header (denoted by \r\n\r\n)
    boost::asio::async_read_until(*socket, *buffer, "\r\n\r\n",
        [socket, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (!error) {
                std::istream stream(buffer.get());
                std::string request_line;
                std::getline(stream, request_line);

                std::cout << "Received request: " << request_line << std::endl;

                std::string response;
                json json_response;

                if (request_line.find("GET /json") != std::string::npos) {
                    // Example of creating a JSON response
                    json_response["message"] = "Hello, JSON!";
                    json_response["status"] = "success";
                    
                    response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " 
                            + std::to_string(json_response.dump().size()) + "\r\n\r\n" 
                            + json_response.dump();
                } else {
                    // Handle other requests
                    response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 14\r\n\r\nRoute Not Found";
                }

                // Write the response back to the client
                boost::asio::async_write(*socket, boost::asio::buffer(response),
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
