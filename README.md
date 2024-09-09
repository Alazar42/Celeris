# Celeris

![Celeris Logo](docs/assets/celeris-main.png)

Celeris is a high-performance backend technology built with C++. Inspired by the Latin word for "fast" or "rapid," Celeris is designed to provide developers with a modern and efficient way to build backend APIs, similar to popular frameworks like FastAPI or Node.js, but with the power and speed of C++.

## Features

- **High Performance**: Leveraging the speed of C++, Celeris provides fast and efficient API handling for high-demand applications.
- **Dynamic Routing**: Easily define routes for your application, supporting both GET and POST methods out of the box.
- **JSON Handling**: Built-in support for JSON serialization and deserialization using the nlohmann JSON library.
- **Extensible**: Modular design allows easy addition of new features and extensions.

## Installation

To get started with Celeris, follow these steps:

1. **Clone the Repository**:

   ```bash
   git clone https://github.com/Alazar42/Celeris.git
   cd Celeris
   ```

2. **Install Dependencies**:
   - **Boost Libraries**: Celeris uses Boost for networking and other core functionalities. You can install Boost using your package manager:
     - **Ubuntu**:

       ```bash
       sudo apt-get install libboost-all-dev
       ```

     - **MacOS** (using Homebrew):

       ```bash
       brew install boost
       ```

     - **Windows**:
       Download and install Boost from the [official Boost website](https://www.boost.org/).

   - **nlohmann JSON Library**: This library handles JSON serialization and deserialization in Celeris.
     - Install via package manager:
       - **Ubuntu**:

         ```bash
         sudo apt-get install nlohmann-json3-dev
         ```

       - **MacOS** (using Homebrew):

         ```bash
         brew install nlohmann-json
         ```

       - **Windows**:
         You can download and add the JSON header file directly from the [nlohmann GitHub repository](https://github.com/nlohmann/json).

3. **Build the Project**:

   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

## Getting Started

Here's a basic example of setting up a simple server with Celeris:

```cpp
#include "celeris/celeris.hpp"
#include <nlohmann/json.hpp>

int main() {
    // Create a Celeris server instance you can set port and host, used for bining purposes
    Celeris app(8080, "127.0.0.1");

    // Define a simple GET route
    app.get("/hello", [](const Request& req, Response& res) {
        nlohmann::json json_response = {{"message", "Hello, world!"}};
        res.set_json(json_response);
    });

    // Start the server
    app.listen();

    return 0;
}
```

## Documentation

For full documentation and advanced usage, please visit the [Celeris Documentation](https://github.com/Alazar42/Celeris/wiki).

## Contributing

Celeris is in active development, and contributions are welcome! To get started:

1. Fork the repository.
2. Create a new branch:

   ```bash
   git checkout -b feature-branch
   ```

3. Make your changes.
4. Submit a pull request.

## License

Celeris is licensed under the GNU General Public License. See the LICENSE file for more information.

## Contact

For questions, issues, or feature requests, please open an issue on GitHub or contact us at <alazartesfaye42@gmail.com>.
