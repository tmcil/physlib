/**
 * Author: Thomas McIlwraith
 */

#include <string.h>

#include "block_sgp4_config.h"

#include "external/cJSON.h"
#include "utils/error_handling.h"
#include "block_common/block_output.h"
#include "product/eph.h"
#include "product/tle.h"
#include "product/tim.h"
#include "environment/environment.h"

int main(int argc, char *argv[]) {
    // Block inputs -----------------------------------------------------------
    // Block config object
    sgp4_config_t block_config      = {0};
    // TLE object
    tle_t  tle                      = {0};
    // Block outputs ----------------------------------------------------------
    // Block output
    block_output_t outputs          = {0};
    // EPH object
    eph_t eph                       = {0};
    // Block variables --------------------------------------------------------
    // Error Status
    err_t err                       = NO_ERROR;
    // Buffer to store the path of each file written
    char *file_written_buf          = NULL;
    // JSON objects
    cJSON *j_block_config = NULL, *j_outputs = NULL;
    cJSON *j_tle = NULL, *j_eph = NULL;

    err = environment_init(argc, argv);
    if (err) return ERR("Failed to initialise working environment");

    // Read the program config
    err = working_dir_read_input(&j_block_config);
    if (err) return ERR("Failed to read json file");
    err = block_sgp4_config_read(&block_config, j_block_config);
    if (err) return ERR("Failed to load config struct");
    cJSON_Delete(j_block_config);

/*     MSG("wgs: %d", block_config.wgs_model);
    epoch_t epoch = {0};
    bool utc = false;
    const char* iso = "2026-12-13T12:30:13.059Z";
    char* iso_out = NULL;

    err = epoch_from_iso8601(&epoch, &utc, iso, MJD);
    if (err) return ERR("Failed to convert epoch string to object");

    err = epoch_to_iso8601(&iso_out, &epoch, utc);
    if (err) return ERR("Failed to convert epoch to iso8601"); */
/*     cJSON *out_json = NULL;
    epoch_t start = {0};
    epoch_t end = {0};
    bool utc;
    const char* iso_start = "2026-12-13T12:30:13.059Z";
    const char* iso_end   = "2026-12-14T12:30:13.059Z";
    epoch_from_iso8601(&start, &utc, iso_start, MJD);
    epoch_from_iso8601(&end, &utc, iso_end, MJD);
    double stepsize = 1;
    tim_t timetest = {0};
    tim_reset(&timetest);
    
    err = tim_init(&timetest, start, end, stepsize);
    if (err) return ERR("Failed to initialise time array");

    err = tim_create_json(&out_json, &timetest, utc);
    if (err) return ERR("Failed to create json file");
    err = working_dir_write(&file_written_buf, OUTPUTS, out_json);
    if (err) return ERR("Failed to write JSON file") */;
    cJSON *json = NULL;
    tim_t test = {0};
    bool utc = false;
    read_json(&json, "/mnt/executions/6ec083b7-faea-4bf7-a62a-b8296cee2238/outputs/BLOCK_SGP4_4902ae5fb193.json");
    err = tim_read_json(&test, &utc, json);
    if (err) return ERR("failed to read json into tim_t");

    MSG("epoch: %f %f %d", test.epochs[0].date1, test.epochs[0].date2, test.epochs[0].format);

    // Free memory
    environment_done();

    return NO_ERROR;
}
