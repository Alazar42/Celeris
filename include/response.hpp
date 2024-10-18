#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include "json.hpp"

// ANSI escape codes for colorful output
#define RESET "\033[0m"
#define RED "\033[31m"

inline void print_debug(const std::string &message, const std::string &color)
{
    std::cout << color << message << RESET << std::endl;
}

class Response {
public:
    int status_code; // HTTP status code (default 200 OK)
    std::string body; // Response body (JSON, HTML, etc.)
    std::map<std::string, std::string> headers; // HTTP headers

    // Constructor with a default status code of 200 (OK)
    Response() : status_code(200) {}

    // Set the response body as a string
    void set_body(const std::string& response_body) {
        body = response_body;
        // Only set Content-Length if it's not already set
        if (headers.find("Content-Length") == headers.end()) {
            set_header("Content-Length", std::to_string(body.size())); // Automatically set content length
        }
    }

    // Set the response body as JSON and update headers accordingly
    void set_json(const nlohmann::json& json_obj) {
        body = json_obj.dump();
        status_code = 200; // Default to 200 OK if set_json is called
        set_header("Content-Type", "application/json");
        // Only set Content-Length if it's not already set
        if (headers.find("Content-Length") == headers.end()) {
            set_header("Content-Length", std::to_string(body.size()));
        }
    }

    // Set an error response with a status code and a detailed message in JSON format
    void set_error(int code, const std::string& detail) {
        nlohmann::json json_response;
        json_response["detail"] = detail;
        json_response["status"] = code;
        body = json_response.dump();
        status_code = code;
        set_header("Content-Type", "application/json");
        
        // Log the error in red
        print_debug("Error " + std::to_string(code) + ": " + detail, RED);

        // Only set Content-Length if it's not already set
        if (headers.find("Content-Length") == headers.end()) {
            set_header("Content-Length", std::to_string(body.size()));
        }
    }

    // Set custom headers (e.g., Content-Type, Content-Length, etc.)
    void set_header(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    // Convert the response to a string format suitable for sending over the network
    std::string to_string() const {
        // HTTP status code to phrase mapping
        static const std::map<int, std::string> status_phrases = {
            {200, "OK"}, {201, "Created"}, {202, "Accepted"}, {204, "No Content"},
            {301, "Moved Permanently"}, {302, "Found"}, {400, "Bad Request"},
            {401, "Unauthorized"}, {403, "Forbidden"}, {404, "Not Found"},
            {405, "Method Not Allowed"}, {500, "Internal Server Error"},
            {503, "Service Unavailable"}
        };

        // Fetch the status phrase or use "Unknown Status" if not found
        auto it = status_phrases.find(status_code);
        std::string status_phrase = (it != status_phrases.end()) ? it->second : "Unknown Status";

        // Build the HTTP response string
        std::ostringstream response_stream;
        response_stream << "HTTP/1.1 " << status_code << " " << status_phrase << "\r\n";

        // Add headers to the response
        for (const auto& [key, value] : headers) {
            response_stream << key << ": " << value << "\r\n";
        }

        // Ensure the Content-Length header is set correctly
        // Only set it if it hasn't been set in the headers
        if (headers.find("Content-Length") == headers.end()) {
            response_stream << "Content-Length: " << body.size() << "\r\n";
        }

        response_stream << "\r\n"; // End of headers
        
        // Append the response body
        response_stream << body;

        // Return the constructed response string
        return response_stream.str();
    }

    // Utility to set a redirect response (for 301 or 302 status codes)
    void set_redirect(int code, const std::string& location) {
        if (code != 301 && code != 302) {
            throw std::invalid_argument("Invalid redirect status code (must be 301 or 302).");
        }
        status_code = code;
        set_header("Location", location);
        body.clear(); // Typically, redirect responses don't have a body
        // Ensure Content-Length is not set for redirect responses
        if (headers.find("Content-Length") == headers.end()) {
            set_header("Content-Length", "0");
        }
    }

    // Utility to set a file download response
    void set_file_response(const std::string& file_data, const std::string& file_name) {
        body = file_data;
        status_code = 200; // OK for file download
        set_header("Content-Type", "application/octet-stream");
        set_header("Content-Disposition", "attachment; filename=\"" + file_name + "\"");
        // Only set Content-Length if it's not already set
        if (headers.find("Content-Length") == headers.end()) {
            set_header("Content-Length", std::to_string(body.size()));
        }
    }
};

#endif // RESPONSE_HPP
