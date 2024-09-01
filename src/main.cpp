#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "router.hpp"
#include "server.hpp"
#include "request.hpp"
#include "response.hpp"

using boost::asio::ip::tcp;
using json = nlohmann::json;

void json_handler(std::shared_ptr<tcp::socket> socket, Request& req, Response& res) {
    json json_response;
    json_response["message"] = "Hello, JSON!";
    json_response["status"] = "success";

    res.status_code = 200;
    res.body = json_response; // Assign the JSON object directly to the body
}

int main() {
    try {
        boost::asio::io_context io_context;
        Server server(io_context, 8080);

        Router::register_route("/json", json_handler);

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
