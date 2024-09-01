#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "celeris.hpp"
#include "request.hpp"
#include "response.hpp"
#include "server.hpp"
#include <nlohmann/json.hpp> // Include this header for JSON handling

using json = nlohmann::json; // Define the json alias for easier usage

// Unified JSON handler for GET and POST requests
void json_handler(std::shared_ptr<tcp::socket> socket, Request& req, Response& res) {
    std::cout << "Received request with method: " << req.method << std::endl;

    if (req.method == "GET") {
        // Handle GET request
        json json_response;
        json_response["message"] = "Hello, this is a GET response!";
        json_response["status"] = "success";

        res.status_code = 200;
        res.body = json_response.dump(); // Convert JSON to string for the response
    } else if (req.method == "POST") {
        // Handle POST request
        std::cout << "Request body: " << req.body << std::endl;

        if (req.body.empty()) {
            // Check if the body is empty and respond accordingly
            std::cerr << "Empty request body received." << std::endl;

            res.status_code = 400;
            json json_response;
            json_response["detail"] = "Empty request body. JSON expected.";
            json_response["status"] = 400;
            res.body = json_response.dump();
            return;
        }

        try {
            json json_request = json::parse(req.body); // Parse the request body as JSON

            std::string message = json_request.value("message", "No message received");

            json json_response;
            json_response["received_message"] = message;
            json_response["status"] = "success";

            res.status_code = 200;
            res.body = json_response.dump(); // Convert JSON to string for the response
        } catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;

            res.status_code = 400;
            json json_response;
            json_response["detail"] = "Invalid JSON format";
            json_response["status"] = 400;
            res.body = json_response.dump();
        }
    } else {
        // Handle unsupported methods
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

        // Register the route with the unified handler for JSON requests
        server.register_route("/json", json_handler);

        server.start();
    } catch (const std::exception& e) {
        // Handle any exceptions thrown during server setup or operation
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
