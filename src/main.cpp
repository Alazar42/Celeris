#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "celeris.hpp"
#include "request.hpp"
#include "response.hpp"
#include "server.hpp"
#include <nlohmann/json.hpp> // Include this header for JSON handling

using json = nlohmann::json; // Define the json alias here

// Handler for POST requests with JSON data
void json_post_handler(std::shared_ptr<tcp::socket> socket, Request& req, Response& res) {
    std::cout << "Received a request with method: " << req.method << std::endl;

    if (req.method == "POST") {
        std::cout << "Request body: " << req.body << std::endl;
        try {
            // Parse the request body as JSON
            json json_request = json::parse(req.body);

            // Extract the "message" field from the JSON, defaulting if not present
            std::string message = json_request.value("message", "No message received");

            // Create a JSON response
            json json_response;
            json_response["received_message"] = message;
            json_response["status"] = "success";

            // Set response status and body
            res.status_code = 200;
            res.body = json_response.dump(); // Convert JSON to string for the response
        } catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;

            // Create an error response for JSON parsing issues
            res.status_code = 400;
            json json_response;
            json_response["detail"] = "Invalid JSON format";
            json_response["status"] = 400;
            res.body = json_response.dump();
        }
    } else {
        // Handle method not allowed cases
        res.status_code = 405;
        json json_response;
        json_response["detail"] = "Method Not Allowed";
        json_response["status"] = 405;
        res.body = json_response.dump();
    }
}

int main() {
    try {
        // Set up the I/O context and server
        boost::asio::io_context io_context;
        Celeris server(io_context, 8080);

        // Register the route for JSON POST requests
        server.register_route("/json", json_post_handler);

        server.start();
    } catch (const std::exception& e) {
        // Handle any exceptions thrown during server setup or operation
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
