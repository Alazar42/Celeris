cmake_minimum_required(VERSION 3.10)
project(Celeris)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set paths for bundled dependencies
set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(BOOST_LIB_DIR ${LIB_DIR})

# Include directories for nlohmann JSON and Boost
include_directories(${INCLUDE_DIR})
include_directories(${INCLUDE_DIR}/boost) # Boost headers

# Manually specify Boost libraries if necessary
set(BOOST_LIBS ${BOOST_LIB_DIR}/libboost_system.a) # Add more Boost libraries as needed

# Check if Boost headers are available
if(EXISTS ${INCLUDE_DIR}/boost)
    message(STATUS "Using bundled Boost: ${INCLUDE_DIR}/boost")
else()
    message(FATAL_ERROR "Bundled Boost not found in ${INCLUDE_DIR}/boost")
endif()


# Add source files
set(SOURCES
    src/main.cpp
)

# Create executable
add_executable(Celeris ${SOURCES})

# Link Boost libraries from the bundled path
target_link_libraries(Celeris ${BOOST_LIBS})

# If additional libraries are needed, specify them here
# e.g., target_link_libraries(Celeris ${BOOST_LIBS} Boost::json)
