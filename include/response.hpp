#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <nlohmann/json.hpp>

class Response {
public:
    int status_code;
    std::string body;
    std::map<std::string, std::string> headers; // Add headers member

    Response() : status_code(200) {}

    // Method to set the response body as JSON
    void set_json(const nlohmann::json& json_obj) {
        body = json_obj.dump(); // Convert JSON object to string
        status_code = 200; // Default status code for success
        headers["Content-Type"] = "application/json"; // Set default header
    }

    // Method to set error JSON response
    void set_error(int code, const std::string& detail) {
        nlohmann::json json_response;
        json_response["detail"] = detail;
        json_response["status"] = code;
        body = json_response.dump();
        status_code = code;
        headers["Content-Type"] = "application/json"; // Set default header
    }

    // Add method to set custom headers
    void set_header(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    std::string to_string() const {
        // Map status codes to reason phrases
        static const std::map<int, std::string> status_phrases = {
            {200, "OK"},
            {400, "Bad Request"},
            {404, "Not Found"},
            {500, "Internal Server Error"}
        };

        // Get the reason phrase for the current status code
        auto it = status_phrases.find(status_code);
        std::string status_phrase = (it != status_phrases.end()) ? it->second : "Unknown Status";

        std::string response;
        response += "HTTP/1.1 " + std::to_string(status_code) + " " + status_phrase + "\r\n";
        
        for (const auto& [key, value] : headers) {
            response += key + ": " + value + "\r\n";
        }
        
        response += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
        response += body;
        
        return response;
    }
};

#endif // RESPONSE_HPP
