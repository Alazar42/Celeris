#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "celeris.hpp"
#include "request.hpp"
#include "server.hpp"
#include "response.hpp"
#include <nlohmann/json.hpp> // Include this header for JSON handling

using json = nlohmann::json; // Define the json alias here

// Define POST request handler
void json_post_handler(std::shared_ptr<tcp::socket> socket, Request& req, Response& res) {
    std::cout << "Received POST request" << std::endl;

    if (req.method == "POST") {
        std::cout << "Request body: " << req.body << std::endl;

        try {
            json json_request = json::parse(req.body);

            std::string message = json_request.value("message", "No message received");

            json json_response;
            json_response["received_message"] = message;
            json_response["status"] = "success";

            res.status_code = 200;
            res.body = json_response.dump(); // Convert json to string
        } catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            res.status_code = 400;
            json json_response;
            json_response["detail"] = "Invalid JSON";
            json_response["status"] = 400;
            res.body = json_response.dump(); // Convert json to string
        }
    } else {
        res.status_code = 405;
        json json_response;
        json_response["detail"] = "Method Not Allowed";
        json_response["status"] = 405;
        res.body = json_response.dump(); // Convert json to string
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        Celeris server(io_context, 8080);

        server.register_route("/json", json_post_handler);

        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
