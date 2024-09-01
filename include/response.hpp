#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <nlohmann/json.hpp>

class Response {
public:
    int status_code;
    std::string content_type;
    std::string body;

    Response() : status_code(200), content_type("text/plain") {}

    void set_json_body(const nlohmann::json& json_body) {
        body = json_body.dump();
        content_type = "application/json";
    }

    std::string to_string() const {
        return "HTTP/1.1 " + std::to_string(status_code) + " OK\r\n" +
               "Content-Type: " + content_type + "\r\n" +
               "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" +
               body;
    }
};

#endif // RESPONSE_HPP
