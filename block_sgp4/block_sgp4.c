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
#include "environment/block_graph.h"

#include "external/SGP4.h"
#include "external/TLE.h"
err_t sgp4_eval(vector_t *teme_vec, const epoch_t epoch, const tle_t tle,
                const wgs_model_t wgs) {
    // Local variables
    err_t err = NO_ERROR;
    TLE satrec = {0};
    double mins_since_epoch = 0.0f;
    char line_1[70];
    char line_2[70];

    vector_reset(teme_vec);
    if (epoch_isreset(epoch)) return ERR("Empty epoch passed to function");
    if (tle_isreset(&tle)) return ERR("Emtpy TLE passed to function");

    memcpy(line_1, tle.line_1, 70);
    memcpy(line_2, tle.line_2, 70);
    memcpy(line_1, tle.line_1, 70);
    memcpy(line_2, tle.line_2, 70);

    parseLines(&satrec, line_1, line_2, (int)wgs);
    if (satrec.sgp4Error != 0) {
        WAR("SGP4 error detected (%d) when parsing TLE for object: %s", satrec.sgp4Error, tle.object_name);
        if (satrec.sgp4Error == 1) WAR("Mean elements, ecc >= 1.0 or ecc < -0.001 or a < 0.95 er");
        if (satrec.sgp4Error == 2) WAR("Mean motion less than 0.0");
        if (satrec.sgp4Error == 3) WAR("Pert elements, ecc < 0.0  or  ecc > 1.0");
        if (satrec.sgp4Error == 4) WAR("Semi-latus rectum < 0.0");
        if (satrec.sgp4Error == 5) WAR("Epoch elements are sub-orbital");
        if (satrec.sgp4Error == 6) WAR("Satellite has decayed");
    }
    mins_since_epoch = (epoch_days(epoch) - (double)satrec.epoch) * 1440;
    getRV(&satrec, mins_since_epoch, &teme_vec->vec[0], &teme_vec->vec[3]);

    teme_vec->epoch = epoch;

    return NO_ERROR;
}

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

/*     // Read the program config
    err = working_dir_read_input(&j_block_config);
    if (err) return ERR("Failed to read json file");
    err = block_sgp4_config_read(&block_config, j_block_config);
    if (err) return ERR("Failed to load config struct");
    cJSON_Delete(j_block_config);

    // Read the TLE input
    err = working_dir_read(&j_tle, "/mnt/executions/6ec083b7-faea-4bf7-a62a-b8296cee2238/files/BLOCK_INPUT_TLE_00_4902ae5fb194.json");
    if (err) return ERR("Failed to read json file");
    err = tle_read_json(&tle, j_tle);
    if (err) return ERR("Failed to load tle struct"); */

    // Read the TIM input
    // ...

    // Evaluate the TLE along TIM with SGP4_eval. Stored in EPH

    // Write the EPH

    // Add EPH to outputs

    // Write the output JSON

/*     vector_t test_vec = {0};
    const char* iso = "2026-10-11T12:00:00.000Z";
    epoch_t test_ep = {0};
    bool utc = true;
    err = epoch_from_iso8601(&test_ep, &utc, iso, J2000);
    if (err) return ERR("Failed to set epoch");
    err = sgp4_eval(&test_vec, test_ep, tle, block_config.wgs_model);
    if (err) return ERR("Failed to compute SV");

    vector_print(test_vec) */;

    // Free memory
    environment_done();

    return NO_ERROR;
}
