cmake_minimum_required(VERSION 3.16)
project(test LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Internal Libraries

add_library(internal_libs STATIC
    block_common/block_output.c
    block_input_tle/block_input_tle_config.c
    block_input_tim/block_input_tim_config.c
    block_sgp4/block_sgp4_config.c

    utils/error_handling.c
    utils/file_io.c
    utils/uuid.c

    environment/environment.c
    environment/parse_cmdline.c
    environment/working_dir.c
    environment/block_graph.c

    product/eph.c
    product/tim.c
    product/tle.c

    tensor/vector.c
    tensor/matrix.c

    time/epoch.c
)
target_include_directories(internal_libs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(internal_libs PUBLIC c_std_23)

# -----------------------------------------------------------------------------
# External Libraries

add_library(cjson STATIC
    external/cJSON.c
    external/cJSON_Utils.c
)
target_include_directories(cjson PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(cjson PUBLIC c_std_23)

add_library(sgp4 STATIC
    external/SGP4.c
    external/TLE.c
)
target_include_directories(sgp4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sgp4 PUBLIC c_std_23)

# Link external libraries to project
target_link_libraries(internal_libs PUBLIC
    sgp4
)

# -----------------------------------------------------------------------------
# Compiler settings

target_compile_features(internal_libs PUBLIC c_std_23)
target_compile_options(internal_libs PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wpedantic
)

# -----------------------------------------------------------------------------
# Linker settings

target_include_directories(internal_libs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# -----------------------------------------------------------------------------
# Create executables

add_executable(block_input_tle
    block_input_tle/block_input_tle.c
)
target_link_libraries(block_input_tle PRIVATE m internal_libs cjson)

add_executable(block_input_tim
    block_input_tim/block_input_tim.c
)
target_link_libraries(block_input_tim PRIVATE m internal_libs cjson)

add_executable(block_sgp4
    block_sgp4/block_sgp4.c
)
target_link_libraries(block_sgp4 PRIVATE m internal_libs cjson)