#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include <iostream> // For debugging
#include <stdexcept> // For runtime_error

class Request {
public:
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;

    Request(const std::string& method, const std::string& path, const std::map<std::string, std::string>& headers, const std::string& body)
        : method(method), path(path), headers(headers), body(body) {}

    // Method to parse the body as JSON and return it
    nlohmann::json parse_json() const {
        // Debugging output for body
        std::cout << "Request Body:\n" << body << std::endl;

        // Check if Content-Type is application/json
        auto content_type_it = headers.find("Content-Type");
        if (content_type_it != headers.end() && content_type_it->second == "application/json") {
            // Attempt to parse the body as JSON
            try {
                return nlohmann::json::parse(body);
            } catch (const nlohmann::json::parse_error& e) {
                // Provide detailed error message when JSON parsing fails
                std::cerr << "JSON Parse Error: " << e.what() << std::endl;
                std::cerr << "Error occurred at line " << e.byte << " in the input." << std::endl;
                throw std::runtime_error("Invalid JSON format: " + std::string(e.what()));
            }
        } else {
            // Optionally handle cases where Content-Type is not application/json
            throw std::runtime_error("Content-Type is not application/json");
        }
    }
};

#endif // REQUEST_HPP
