// response.hpp
#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <nlohmann/json.hpp>

class Response {
public:
    int status_code;
    std::string body;

    Response() : status_code(200) {}

    // Method to set the response body as JSON
    void set_json(const nlohmann::json& json_obj) {
        body = json_obj.dump(); // Convert JSON object to string
        status_code = 200;
    }

    // Method to set error JSON response
    void set_error(int code, const std::string& detail) {
        nlohmann::json json_response;
        json_response["detail"] = detail;
        json_response["status"] = code;
        body = json_response.dump();
        status_code = code;
    }

    std::string to_string() const {
        return "HTTP/1.1 " + std::to_string(status_code) + " OK\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
    }
};

#endif // RESPONSE_HPP