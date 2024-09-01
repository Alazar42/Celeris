//request.hpp

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <nlohmann/json.hpp>

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
        if (headers.find("Content-Type") != headers.end() && headers.at("Content-Type") == "application/json") {
            try {
                return nlohmann::json::parse(body);
            } catch (const nlohmann::json::parse_error& e) {
                throw std::runtime_error("Invalid JSON format: " + std::string(e.what()));
            }
        } else {
            throw std::runtime_error("Content-Type is not application/json");
        }
    }
};

#endif // REQUEST_HPP