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

    // Set the response body as JSON
    void set_json(const nlohmann::json& json_obj) {
        body = json_obj.dump();
        status_code = 200;
        headers["Content-Type"] = "application/json";
    }

    // Set error JSON response
    void set_error(int code, const std::string& detail) {
        nlohmann::json json_response;
        json_response["detail"] = detail;
        json_response["status"] = code;
        body = json_response.dump();
        status_code = code;
        headers["Content-Type"] = "application/json";
    }

    // Set custom headers
    void set_header(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    // Convert response to string for sending over the network
    std::string to_string() const {
        static const std::map<int, std::string> status_phrases = {
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {204, "No Content"},
    {206, "Partial Content"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {304, "Not Modified"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {409, "Conflict"},
    {413, "Request Entity Too Large"},
    {414, "Request-URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Requested Range Not Satisfiable"},
    {417, "Expectation Failed"},
    {418, "I'm a teapot"}, // A humorous status code
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"}
};
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
