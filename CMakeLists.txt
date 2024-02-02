cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(serverFW VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if(NOT CMAKE_CXX_COMPILE_FEATURES)
    message(FATAL_ERROR "The compiler does not support C++17.")
endif()

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

set(FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR "/usr")
include_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/include)
link_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/lib)

add_executable(${PROJECT_NAME}
    "app/main.cpp"
    "fw/server/global.cpp"
    "fw/server/server.cpp"
    "fw/server/server_in.cpp"
)