cmake_minimum_required(VERSION 3.10)

# Set the project name
project(LSPTSearchEngine)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Collect all source files in the current directory and subdirectories
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable
add_executable(search_engine ${SOURCES})

# Link necessary libraries (adjust as per your requirements)
# target_link_libraries(search_engine PUBLIC <library_name>)