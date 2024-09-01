#include "router.hpp"
#include "request.hpp"
#include "server.hpp"
#include "response.hpp"
#include <iostream>
#include <nlohmann/json.hpp> // Ensure you include the JSON library

using json = nlohmann::json;

void json_handler(std::shared_ptr<tcp::socket> socket, Request& req, Response& res) {
    // Create a JSON response
    json json_response;
    json_response["message"] = "Hello, JSON!";
    json_response["status"] = "success";

    res.set_json_body(json_response);
}

int main() {
    try {
        boost::asio::io_context io_context;
        Server server(io_context, 8080);

        // Register routes
        Router::register_route("/json", json_handler);

        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
