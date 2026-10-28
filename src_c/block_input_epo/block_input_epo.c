/**
 * Author: Thomas McIlwraith
 */

#include "block_input_epo_config.h"

#include "block_common/block_output.h"
#include "product/epo.h"
#include "environment/environment.h"
#include "external/cJSON.h"
#include "core/error_handling.h"


/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[]) {
    err_t err                       = NO_ERROR;       //     Error code
    input_epo_config_t block_config = {0};            // MEM Block input config
    output_block_t outputs          = {0};            // MEM Block output data
    interval_t timespan             = {0};            // MEM Block output interval
    epoch_t start                   = {0};            //     Start epoch
    epoch_t end                     = {0};            //     End epoch
    char *file_written_buf          = NULL;           // MEM Buffer which stores filenames written
    cJSON *j_block_config = NULL, *j_outputs = NULL;  // MEM Block JSONs
    cJSON *j_epo = NULL;                              // MEM Product JSONs

    err = environment_init(argc, argv);
    if (err) return ERR("Failed to initialise working environment");

    // Read the program config
    err = working_dir_read_input(&j_block_config);
    if (err) return ERR("Failed to read input JSON file");
    err = input_epo_config_read_json(&block_config, j_block_config);
    if (err) return ERR("Input JSON file is malformed");

    // Load the start/end times into epochs
    err = epoch_from_iso8601(&start, block_config.start);
    if (err) return ERR("Failed to convert start date to epoch: %s", block_config.start);
    err = epoch_from_iso8601(&end, block_config.end);
    if (err) return ERR("Failed to convert end date to epoch: %s", block_config.end);

    MSG("Start epoch: %s", block_config.start);
    MSG("End epoch  : %s", block_config.end);
    MSG("Generating an interval with timestep: %02f seconds", block_config.stepsize_seconds);

    err = interval_linear(&timespan, start, end, delta_seconds(block_config.stepsize_seconds));
    if (err) return ERR("Failed to compute time interval");
    
    MSG("Successfully generated %d datapoints", (int)timespan.n_epochs);

    // Create the product JSON
    epo_create_json(&j_epo, &timespan);
    if (err) return ERR("Failed to convert product EPO to JSON");
    err = working_dir_write(&file_written_buf, FILES, j_epo);
    if (err) return ERR("Failed to write TIM JSON file");

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

    // Free memory
    environment_done();
    input_epo_config_free(&block_config);
    output_block_reset(&outputs);
    interval_free(&timespan);
    if (file_written_buf != NULL) free(file_written_buf);
    cJSON_Delete(j_block_config); cJSON_Delete(j_outputs);
    cJSON_Delete(j_epo);

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */
