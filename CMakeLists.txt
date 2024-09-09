cmake_minimum_required(VERSION 3.10)
project(Celeris)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include FetchContent for automatic dependency management
include(FetchContent)

# Boost
FetchContent_Declare(
    boost
    URL https://boostorg.jfrog.io/artifactory/main/release/1.74.0/source/boost_1_74_0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(boost)

# nlohmann JSON
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2  # specify the version you need
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(nlohmann_json)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${nlohmann_json_SOURCE_DIR}/include)

# Add source files
set(SOURCES
    src/main.cpp
    src/celeris.cpp
    src/router.cpp
)

# Create executable
add_executable(Celeris ${SOURCES})

# Link Boost libraries
target_link_libraries(Celeris Boost::system nlohmann_json::nlohmann_json)
