// response.hpp

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <nlohmann/json.hpp>  // Include nlohmann JSON library

using json = nlohmann::json;

class Response {
public:
    int status_code;
    json body;

    Response() : status_code(200) {}

    std::string to_string() const {
        std::string status_line = "HTTP/1.1 " + std::to_string(status_code) + " OK\r\n";
        std::string headers = "Content-Type: application/json\r\nContent-Length: " + std::to_string(body.dump().size()) + "\r\n\r\n";
        return status_line + headers + body.dump();
    }
};

#endif // RESPONSE_HPP
