#include <boost/asio.hpp>
#include <iostream>
#include "server.hpp"

int main() {
    try {
        boost::asio::io_context io_context;
        Server server(io_context, 8080);  // Start server on port 8080
        io_context.run();                 // Run the IO context
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
