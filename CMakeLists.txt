cmake_minimum_required(VERSION 3.16)
project(physlib LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)


# Set executable output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)

target_compile_features(internal_libs PUBLIC c_std_23)
target_compile_options(internal_libs PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wpedantic
)

target_include_directories(internal_libs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})