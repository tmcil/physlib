/**
 * Author: Thomas McIlwraith
 */

#include "block_input_tim_config.h"

#include "block_common/block_output.h"
#include "product/tim.h"
#include "environment/environment.h"
#include "external/cJSON.h"
#include "utils/error_handling.h"

int main(int argc, char *argv[]) {
    // Block inputs -----------------------------------------------------------
    input_tim_config_t block_config = {0};
    // Block outputs ----------------------------------------------------------
    output_block_t outputs          = {0};
    tim_t timespan                  = {0};
    // Block variables --------------------------------------------------------
    err_t err                       = NO_ERROR;
    epoch_t start                   = {0};
    epoch_t end                     = {0};
    double aux_stepsize             = 0.0;
    bool utc                        = true;
    char *file_written_buf          = NULL;
    cJSON *j_block_config = NULL, *j_outputs = NULL, *j_tim = NULL;

    err = environment_init(argc, argv);
    if (err) return ERR("Failed to initialise working environment");

    // Read the program config
    err = working_dir_read_input(&j_block_config);
    if (err) return ERR("Failed to read input JSON file");
    err = input_tim_config_read_json(&block_config, j_block_config);
    if (err) return ERR("Input JSON file is malformed");
    cJSON_Delete(j_block_config);

    // Start time is always present
    err = epoch_from_iso8601(&start, &utc, block_config.start, J2000);
    if (err) return ERR("Failed to convert start date to epoch: %s", block_config.start);

    // End time may be == start time, if only a single point is required
    if (block_config.is_single) {
        end = start;
        aux_stepsize = 86400;
    } else {
        err = epoch_from_iso8601(&end, &utc, block_config.end, J2000);
        if (err) return ERR("Failed to convert end date to epoch: %s", block_config.end);
        aux_stepsize = block_config.stepsize_seconds;
    }

    err = tim_init(&timespan, start, end, aux_stepsize);
    if (err) return ERR("Failed to compute timespan");
    
    MSG("Input start: %s", block_config.start);
    MSG("Input end:   %s", block_config.start);
    MSG("Generated %d epochs with a stepsize of %f seconds", timespan.n_epochs, aux_stepsize);

    // Create the product JSON
    err = tim_create_json(&j_tim, &timespan, utc);
    if (err) return ERR("Failed to convert product TIM to JSON");
    err = working_dir_write(&file_written_buf, FILES, j_tim);
    if (err) return ERR("Failed to write TIM JSON file");
    cJSON_Delete(j_tim);

    // Initialise the output
    err = output_block_init(&outputs);
    if (err) return ERR("Failed to initialise the output block for %s", g_cmdline_args.block_name);
    err = output_block_push(&outputs, "interval", file_written_buf);
    if (err) return ERR("Failed to add TLE to the outputs");

    // Write the block output
    err = output_block_create_json(&j_outputs, &outputs);
    if (err) return ERR("Failed to convert outputs to JSON");
    err = working_dir_write(&file_written_buf, OUTPUTS, j_outputs);
    if (err) return ERR("Failed to write output file");
    cJSON_Delete(j_outputs);

    // Free memory
    environment_done();
    tim_reset(&timespan);
    if (file_written_buf != NULL) free(file_written_buf);

    return NO_ERROR;
}
