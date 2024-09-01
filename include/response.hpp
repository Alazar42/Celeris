// response.hpp

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include "nlohmann/json.hpp"

class Response {
public:
    int status_code;
    nlohmann::json body;

    std::string to_string() const {
        std::string status_line = "HTTP/1.1 " + std::to_string(status_code) + " OK\r\n";
        std::string headers = "Content-Type: application/json\r\n";
        std::string body_str = body.dump();
        std::string response = status_line + headers + "Content-Length: " + std::to_string(body_str.size()) + "\r\n\r\n" + body_str;
        return response;
    }
};

#endif // RESPONSE_HPP
