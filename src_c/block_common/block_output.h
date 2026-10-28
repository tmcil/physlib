#ifndef BLOCK_OUTPUT_H
#define BLOCK_OUTPUT_H

/**
 * Author: Thomas McIlwraith
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "core/error_handling.h"
#include "utils/uuid.h"
#include "external/cJSON.h"
#include "environment/block_graph.h"
#include "environment/environment.h"


typedef struct {
    char* port_id;
    char* filepath;
} output_port_t;

typedef struct {
    char *block;
    uuid_t block_id;
    output_port_t *ports;
    size_t n_ports;
} output_block_t;


/* ------------------------------------------------------------------------- */

err_t output_block_read_json(output_block_t *item, const cJSON *json);

/* ------------------------------------------------------------------------- */

err_t output_block_create_json(cJSON **json, const output_block_t *item);

/* ------------------------------------------------------------------------- */

err_t output_block_push(output_block_t *item, const char* port, const char* path);

/* ------------------------------------------------------------------------- */

err_t output_block_init(output_block_t *item);

/* ------------------------------------------------------------------------- */

err_t output_block_copy(output_block_t *dst, const output_block_t src);

/* ------------------------------------------------------------------------- */

void output_block_reset(output_block_t *item);

/* ------------------------------------------------------------------------- */

void output_port_reset(output_port_t *item);

/* ------------------------------------------------------------------------- */

#endif // BLOCK_OUTPUT_H
