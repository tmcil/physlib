/**
 * Author: Thomas McIlwraith
 */

#include "block_input_tle_config.h"

#include "block_common/block_output.h"
#include "datastructures/tle.h"
#include "environment/environment.h"
#include "external/cJSON.h"
#include "utils/error_handling.h"

int main(int argc, char *argv[]) {
    // Block inputs -----------------------------------------------------------
    // Block config
    input_tle_config_t block_config = {0};
    // Block config JSON
    cJSON *block_config_json        = NULL;
    // Block outputs ----------------------------------------------------------
    // Block output
    block_output_t outputs          = {0};
    // Block output JSON
    cJSON *outputs_json             = NULL;
    // TLE object
    tle_t  tle                      = {0};
    // TLE object JSON
    cJSON *tle_json                 = NULL;
    // Block variables --------------------------------------------------------
    // Error Status
    err_t err                       = NO_ERROR;
    // Buffer to store the path of each file written
    char *file_written_buf          = NULL;

    err = environment_init(argc, argv);
    if (err) return ERR("Failed to initialise working environment");

    // Read the program config
    err = working_dir_read_input(&block_config_json);
    if (err) return ERR("Failed to read json file");
    err = input_tle_config_read_json(&block_config, block_config_json);
    if (err) return ERR("Failed to load config struct");
    cJSON_Delete(block_config_json);

    // Fill the TLE type
    tle.satellite_id = block_config.satellite_id;
    tle.object_name = block_config.object_name; // TODO, load this from db
    snprintf(tle.line_1, sizeof(tle.line_1), "%s", block_config.line_1);
    snprintf(tle.line_2, sizeof(tle.line_2), "%s", block_config.line_2);
    MSG("Successfully created TLE for: %s", tle.object_name);

    // Create the output JSONs
    err = tle_create_json(&tle_json, &tle);
    if (err) return ERR("Failed to create TLE JSON");
    err = working_dir_write(&file_written_buf, FILES, tle_json);
    if (err) return ERR("Failed to write TLE file");
    cJSON_Delete(tle_json);

    // Push the written file into the outputs object
    err = block_output_push(&outputs, OUTPUT_TLE, file_written_buf);
    if (err) return ERR("Failed to fill output struct");

    // Write the block output
    err = block_output_create_json(&outputs_json, &outputs);
    if (err) return ERR("Failed to create output JSON");
    err = working_dir_write(&file_written_buf, OUTPUTS, outputs_json);
    if (err) return ERR("Failed to write output file");
    cJSON_Delete(outputs_json);

    // Free memory
    environment_done();
    tle_reset(&tle);
    block_output_reset(&outputs);
    if (file_written_buf != NULL) free(file_written_buf);

    return NO_ERROR;
}
