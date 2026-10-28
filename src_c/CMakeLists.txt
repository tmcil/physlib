# -----------------------------------------------------------------------------
# Internal Libraries

add_library(internal_libs STATIC
    # Functionality common to all blocks
    block_common/block_output.c
    
    # Input Blocks
    block_input_tle/block_input_tle_config.c
    block_input_epo/block_input_epo_config.c
    
    # Process Blocks
    block_sgp4/block_sgp4_config.c

    # Output Blocks

    # Utilities
    utils/file_io.c
    utils/uuid.c

    # Environment handling
    environment/environment.c
    environment/parse_cmdline.c
    environment/working_dir.c
    environment/block_graph.c

    # Product datatypes and methods
    product/eph.c
    product/epo.c
    product/tle.c
    
    # Core libraries
    core/error_handling.c
    core/epoch.c
    core/vector.c
    core/matrix.c
    core/cartsv.c

)
target_include_directories(internal_libs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(internal_libs PUBLIC c_std_23)

# -----------------------------------------------------------------------------
# External Libraries

add_library(cjson STATIC
    external/cJSON/cJSON.c
    external/cJSON/cJSON_Utils.c
)
target_include_directories(cjson PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(cjson PUBLIC c_std_23)

add_library(sgp4 STATIC
    external/SGP4/SGP4.c
    external/SGP4/TLE.c
)
target_include_directories(sgp4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sgp4 PUBLIC c_std_23)

file(GLOB SOFA_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/external/sofa/*.c")
add_library(sofa STATIC ${SOFA_SOURCES})
target_include_directories(sofa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sofa PUBLIC c_std_23)

# Link external libraries to project
target_link_libraries(internal_libs PUBLIC
    sgp4
    m
)

# -----------------------------------------------------------------------------
# Create executables

add_executable(block_input_tle
    block_input_tle/block_input_tle.c
)
target_link_libraries(block_input_tle PRIVATE m internal_libs cjson)

add_executable(block_input_epo
    block_input_epo/block_input_epo.c
)
target_link_libraries(block_input_epo PRIVATE m internal_libs cjson)

add_executable(block_sgp4
    block_sgp4/block_sgp4.c
)
target_link_libraries(block_sgp4 PRIVATE m internal_libs cjson)