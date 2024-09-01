#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class Request {
public:
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;

    Request(const std::string& method, const std::string& path, 
            const std::map<std::string, std::string>& headers, const std::string& body)
        : method(method), path(path), headers(headers), body(body) {}
};

#endif // REQUEST_HPP
