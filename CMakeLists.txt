cmake_minimum_required(VERSION 3.10)

# Set the project name
project(LSPTSearchEngine)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include MongoDB C++ Driver
find_package(PkgConfig REQUIRED)
find_package(Crow)

pkg_check_modules(MONGOCXX REQUIRED libmongocxx)

# Set the compile and linker flags directly from pkg-config
set(MONGOCXX_CFLAGS ${MONGOCXX_CFLAGS_OTHER})
set(MONGOCXX_LDFLAGS ${MONGOCXX_LIBRARIES})

# Collect all source files in the current directory and subdirectories
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable
add_executable(search_engine ${SOURCES})

# Apply the compile flags
target_compile_options(search_engine PRIVATE ${MONGOCXX_CFLAGS})
#target_compile_options(search_engine PRIVATE -fconcepts)
# Link the libraries
target_link_libraries(search_engine PRIVATE ${MONGOCXX_LDFLAGS})
target_link_libraries(search_engine PUBLIC Crow::Crow)

