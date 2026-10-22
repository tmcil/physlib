/**
 * Author: Thomas McIlwraith
 */

#include "block_input_tle_config.h"

#include "block_common/block_output.h"
#include "product/tle.h"
#include "environment/environment.h"
#include "external/cJSON.h"
#include "utils/error_handling.h"

int main(int argc, char *argv[]) {
    // Block inputs -----------------------------------------------------------
    input_tle_config_t block_config = {0};
    // Block outputs ----------------------------------------------------------
    output_block_t outputs          = {0};
    tle_t  tle                      = {0};
    // Block variables --------------------------------------------------------
    err_t err                       = NO_ERROR;
    char *file_written_buf          = NULL;
    cJSON *j_block_config = NULL, *j_outputs = NULL, *j_tle = NULL;

    err = environment_init(argc, argv);
    if (err) return ERR("Failed to initialise working environment");

    // Read the program config
    err = working_dir_read_input(&j_block_config);
    if (err) return ERR("Failed to read input JSON file");
    err = input_tle_config_read_json(&block_config, j_block_config);
    if (err) return ERR("Input JSON file is malformed");
    cJSON_Delete(j_block_config);

    // Fill the TLE type
    snprintf(tle.line_1, sizeof(tle.line_1), "%s", block_config.line_1);
    snprintf(tle.line_2, sizeof(tle.line_2), "%s", block_config.line_2);

    MSG(" Input TLE[1]: %s", tle.line_1);
    MSG(" Input TLE[2]: %s", tle.line_2);

    // Create the product JSON
    err = tle_create_json(&j_tle, &tle);
    if (err) return ERR("Failed to convert product TLE to JSON");
    err = working_dir_write(&file_written_buf, FILES, j_tle);
    if (err) return ERR("Failed to write TLE JSON file");
    cJSON_Delete(j_tle);

    // Initialise the output object
    err = output_block_init(&outputs);
    if (err) return ERR("Failed to initialise the output block for %s", g_cmdline_args.block_name);
    // Push the TLE to the output port
    err = output_block_push(&outputs, "tle", file_written_buf);
    if (err) return ERR("Failed to add TLE to the outputs");

    // Write the block output
    err = output_block_create_json(&j_outputs, &outputs);
    if (err) return ERR("Failed to convert outputs to JSON");
    err = working_dir_write(&file_written_buf, OUTPUTS, j_outputs);
    if (err) return ERR("Failed to write output file");
    cJSON_Delete(j_outputs);

    MSG("TLE successfully created");

    // Free memory
    environment_done();
    tle_reset(&tle);
    if (file_written_buf != NULL) free(file_written_buf);

    return NO_ERROR;
}
