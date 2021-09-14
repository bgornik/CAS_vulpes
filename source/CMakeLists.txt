add_library(bignum bignum.cpp)
add_library(functions functions.cpp)
add_library(expression expression.cpp)
target_include_directories(bignum PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/source"
                          "${PROJECT_SOURCE_DIR}/include"
                          )
target_include_directories(functions PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/source"
                          "${PROJECT_SOURCE_DIR}/include"
                          )
target_include_directories(expression PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/source"
                          "${PROJECT_SOURCE_DIR}/include"
                          )
