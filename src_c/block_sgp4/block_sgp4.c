/**
 * Author: Thomas McIlwraith
 */

#include <string.h>

#include "block_sgp4_config.h"

#include "external/cJSON.h"
#include "core/error_handling.h"
#include "block_common/block_output.h"
#include "product/eph.h"
#include "product/tle.h"
#include "product/epo.h"
#include "environment/environment.h"
#include "environment/block_graph.h"

#include "external/SGP4.h"
#include "external/TLE.h"
err_t sgp4_eval(cartsv_t *teme_vec, const epoch_t epoch, const tle_t tle,
                const wgs_model_t wgs) {
    // Local variables
    err_t err = NO_ERROR;
    TLE satrec = {0};
    epoch_t satrec_epoch = {0};
    double mins_since_epoch = 0.0f;
    char line_1[70];
    char line_2[70];

    if (epoch_iszerod(epoch)) return ERR("Empty epoch passed to function");
    if (tle_isreset(&tle)) return ERR("Emtpy TLE passed to function");

    memcpy(line_1, tle.line_1, 70);
    memcpy(line_2, tle.line_2, 70);
    memcpy(line_1, tle.line_1, 70);
    memcpy(line_2, tle.line_2, 70);

    parseLines(&satrec, line_1, line_2, (int)wgs);
    if (satrec.sgp4Error != 0) {
        WAR("SGP4 error detected (%d) when parsing TLE for object", satrec.sgp4Error);
        if (satrec.sgp4Error == 1) WAR("Mean elements, ecc >= 1.0 or ecc < -0.001 or a < 0.95 er");
        if (satrec.sgp4Error == 2) WAR("Mean motion less than 0.0");
        if (satrec.sgp4Error == 3) WAR("Pert elements, ecc < 0.0  or  ecc > 1.0");
        if (satrec.sgp4Error == 4) WAR("Semi-latus rectum < 0.0");
        if (satrec.sgp4Error == 5) WAR("Epoch elements are sub-orbital");
        if (satrec.sgp4Error == 6) WAR("Satellite has decayed");
    }

    epoch_zero(&satrec_epoch);
    satrec_epoch.format = JD;
    satrec_epoch.scale = UTC;
    satrec_epoch.date1 = (double)satrec.epoch;
    satrec_epoch.date2 = 0.0;
    epoch_switch_format(&satrec_epoch, J2000);
    epoch_delta_t delta = delta_from_epochs(satrec_epoch, epoch);

    mins_since_epoch = delta_to_days(delta) * 1440.0;
    getRV(&satrec, mins_since_epoch, teme_vec->r.values, teme_vec->v.values);

    teme_vec->epoch = epoch;
    teme_vec->frame = ORBIT_FRAME_TEME;

    return NO_ERROR;
}

int main(int argc, char *argv[]) {
    // Block inputs -----------------------------------------------------------
    sgp4_config_t block_config = {0};
    interval_t timespan        = {0};
    tle_t tle                  = {0};
    // Block outputs ----------------------------------------------------------
    output_block_t outputs     = {0};
    cartsv_array_t eph         = {0};
    // Block variables --------------------------------------------------------
    err_t err                  = NO_ERROR;
    char *file_written_buf     = NULL;
    size_t i                   = 0;
    cartsv_t statevector       = {0};
    // JSON objects -----------------------------------------------------------
    cJSON *j_block_config = NULL, *j_outputs = NULL;
    cJSON *j_tle = NULL, *j_epo = NULL, *j_eph = NULL;

    err = environment_init(argc, argv);
    if (err) return ERR("Failed to initialise working environment");

    err = output_block_init(&outputs);
    if (err) return ERR("Failed to initialise output object");

     // Read the program config
    err = working_dir_read_input(&j_block_config);
    if (err) return ERR("Failed to read config JSON file");
    err = block_sgp4_config_read(&block_config, j_block_config);
    if (err) return ERR("Failed to load config object");
    cJSON_Delete(j_block_config);

    // Read the TLE input
    err = graph_find_input_port_file(&j_tle, g_cmdline_args.block_id, "tle");
    if (err) return ERR("Failed to find file at input port: tle");
    err = tle_read_json(&tle, j_tle);
    if (err) return ERR("Failed to load tle struct");

    // Read the EPO input
    err = graph_find_input_port_file(&j_epo, g_cmdline_args.block_id, "interval");
    if (err) return ERR("Failed to find file at input port: tim");
    err = epo_read_json(&timespan, j_epo);
    if (err) return ERR("Failed to parse EPO json");

    if (timespan.n_epochs < 1) return ERR("Empty EPO interval");
    MSG("Generating an EPH with %d datapoints", (int)timespan.n_epochs);

    err = cartsv_array_init(&eph, timespan.n_epochs);
    if (err) return ERR("Failed to inialise EPH product");

    // Evaluate the TLE along EPO with SGP4_eval. Stored in EPH
    // err = eph_init(&eph, 0, 0, 1000.0); // SGP4 outputs in km

    // TODO: Rotate this to actually be in J2000, currently in TEME
    statevector.frame = ORBIT_FRAME_J2000;
    for (i = 0; i<timespan.n_epochs; i++) {
        epoch_copy(&statevector.epoch, timespan.epochs[i]);
        vector_init(&statevector.r, 3);
        vector_init(&statevector.v, 3);
        matrix_init_identity(&statevector.cov, 6);

        err = sgp4_eval(&statevector, timespan.epochs[i], tle, block_config.wgs_model);
        if (err) return ERR("Failed to compute state vector at epoch [%d]", (int)i);

        err = cartsv_array_push(&eph, statevector);
        if (err) return ERR("Failed to add state vector[%d] to EPH product", (int)i);
    }

    // Create the EPH product JSON
    err = eph_create_json(&j_eph, &eph);
    if (err) return ERR("Failed to convert product EPH to JSON");
    err = working_dir_write(&file_written_buf, FILES, j_eph);
    if (err) return ERR("Failed to write EPH JSON file");
    cJSON_Delete(j_tle);

    // Initialise the output object
    err = output_block_init(&outputs);
    if (err) return ERR("Failed to initialise the output block for %s", g_cmdline_args.block_name);
    // Push the EPH to the output port
    err = output_block_push(&outputs, "ephemeris", file_written_buf);
    if (err) return ERR("Failed to add EPH to the outputs");

    // Write the block output
    err = output_block_create_json(&j_outputs, &outputs);
    if (err) return ERR("Failed to convert outputs to JSON");
    err = working_dir_write(&file_written_buf, OUTPUTS, j_outputs);
    if (err) return ERR("Failed to write output file");
    cJSON_Delete(j_outputs);

    // Free memory
    environment_done();
    cartsv_array_free(&eph);
    interval_free(&timespan);

    return NO_ERROR;
}
