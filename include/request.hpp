#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include <iostream>
#include <stdexcept>

// Custom Request class for handling HTTP request data
class Request {
public:
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;

    // Constructor
    Request(const std::string& method, const std::string& path,
            const std::map<std::string, std::string>& headers, const std::string& body)
        : method(method), path(path), headers(headers), body(body) {}

    // Method to parse the body as JSON
    nlohmann::json parse_json() const {
        // Check if the body is empty
        if (body.empty()) {
            throw std::runtime_error("Request body is empty; cannot parse JSON.");
        }

        // Ensure Content-Type is application/json
        auto content_type_it = headers.find("Content-Type");
        if (content_type_it == headers.end() || content_type_it->second != "application/json") {
            throw std::runtime_error("Content-Type is not application/json.");
        }

        try {
            // Attempt to parse the JSON
            return nlohmann::json::parse(body);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
            throw std::runtime_error("Invalid JSON format: " + std::string(e.what()));
        }
    }

    // Method to display request details for debugging purposes
    void inspect() const {
        std::cout << "Method: " << method << "\nPath: " << path << "\nHeaders:\n";
        for (const auto& [key, value] : headers) {
            std::cout << key << ": " << value << "\n";
        }
        std::cout << "Body:\n" << body << std::endl;
    }
};

#endif // REQUEST_HPP
