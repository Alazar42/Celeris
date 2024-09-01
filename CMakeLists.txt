cmake_minimum_required(VERSION 3.10)
project(Celeris)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Boost libraries
find_package(Boost 1.74 REQUIRED COMPONENTS system)

include_directories(${PROJECT_SOURCE_DIR}/include)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Boost not found")
endif()

add_executable(Celeris src/main.cpp src/router.cpp src/server.cpp)

# Link Boost libraries
target_link_libraries(Celeris Boost::system)
