
cmake_minimum_required(VERSION 3.10)

# set the project name
project(Solve)

# add the libraries
add_subdirectory(source)

# add the executable
add_executable(Solve solve.cpp)

target_link_libraries(Solve expression functions bignum)


# add the binary tree to the search path for include files
target_include_directories(Solve PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/include"
                          )
                          

