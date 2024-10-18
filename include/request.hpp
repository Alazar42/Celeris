#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include "json.hpp"
#include <iostream>
#include <stdexcept>
#include <algorithm>

// Custom Request class for handling HTTP request data
class Request
{
public:
    // Public member variables for easy access
    std::string method_;
    std::string path_;
    std::map<std::string, std::string> headers_;
    std::string body_;

    // Constructor
    Request(const std::string &method, const std::string &path, const std::map<std::string, std::string> &headers, const std::string &body)
        : method_(method), path_(normalize_path(path)), headers_(normalize_headers(headers)), body_(body) {}

    // Getter for the request method
    std::string get_method() const
    {
        return method_;
    }

    // Getter for the request path
    std::string get_path() const
    {
        return path_;
    }

    // Method to get a specific header value (returns empty string if not found)
    std::string get_header(const std::string &header_name) const
    {
        auto it = headers_.find(header_name);
        if (it != headers_.end())
        {
            return it->second;
        }
        return "";
    }

    // Method to check if a specific header exists
    bool has_header(const std::string &header_name) const
    {
        return headers_.find(header_name) != headers_.end();
    }

    // Method to parse the body as JSON
    nlohmann::json parse_json() const
    {
        // Check if the body is empty
        if (body_.empty())
        {
            // Return an empty JSON object instead of throwing an exception
            std::cout << "Warning: Request body is empty; returning empty JSON." << std::endl;
            return nlohmann::json(); // Return an empty JSON object
        }

        // Ensure Content-Type is application/json
        auto content_type_it = headers_.find("Content-Type");
        if (content_type_it == headers_.end() || content_type_it->second != "application/json")
        {
            throw std::runtime_error(format_error_message("Content-Type is not application/json."));
        }

        try
        {
            // Attempt to parse the JSON
            return nlohmann::json::parse(body_);
        }
        catch (const nlohmann::json::parse_error &e)
        {
            std::cerr << format_error_message("JSON Parse Error: " + std::string(e.what())) << std::endl;
            throw std::runtime_error("Invalid JSON format: " + std::string(e.what()));
        }
    }

    // Method to display request details for debugging purposes
    void inspect() const
    {
        std::cout << "Method: " << method_ << "\nPath: " << path_ << "\nHeaders:\n";
        for (const auto &[key, value] : headers_)
        {
            std::cout << key << ": " << value << "\n";
        }
        std::cout << "Body:\n" << body_ << std::endl;
    }

private:
    // Normalize headers to ensure case-insensitivity
    static std::map<std::string, std::string> normalize_headers(const std::map<std::string, std::string> &headers)
    {
        std::map<std::string, std::string> normalized_headers;
        for (const auto &[key, value] : headers)
        {
            std::string lowercase_key = key;
            std::transform(lowercase_key.begin(), lowercase_key.end(), lowercase_key.begin(), ::tolower);
            normalized_headers[lowercase_key] = value;
        }
        return normalized_headers;
    }

    // Normalize path by ensuring there's no trailing slash
    static std::string normalize_path(const std::string &path)
    {
        if (path.length() > 1 && path.back() == '/')
        {
            return path.substr(0, path.length() - 1);
        }
        return path;
    }

    // Format error message to display in red
    static std::string format_error_message(const std::string &message)
    {
        return "\033[31m" + message + "\033[0m"; // ANSI escape code for red text
    }
};

#endif // REQUEST_HPP
