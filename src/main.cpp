#include "celeris.hpp"

int main() {
    // Create a Celeris server instance
    Celeris app(8080, "127.0.0.1");

    // Define a simple GET route
    app.get("/hello", [](const Request& req, Response& res) {
        nlohmann::json json_response = {{"message", "Hello, world!"}};
        res.set_json(json_response);
    });

    app.get("/", [](const Request& req, Response& res) {
        nlohmann::json json_response = {{"message", "Welcome To Celris Backend"}};
        res.set_json(json_response);
    });

    // Define a POST route
    app.post("/echo", [](const Request& req, Response& res) {
        nlohmann::json json_request = nlohmann::json::parse(req.body_);
        res.set_json(json_request);
    });

    // Start the server
    app.listen();

    return 0;
}
