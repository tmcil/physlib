#ifndef BLOCK_GRAPH_H
#define BLOCK_GRAPH_H

/**
 * Author: Thomas McIlwraith
 */

#include "external/cJSON.h"
#include "utils/error_handling.h"
#include "utils/uuid.h"

// Module data structures -----------------------------------------------------

typedef struct {
    uuid_t block_id;
    char *port_id;
} graph_connection_t;

typedef struct {
    char *port_id;
    char *type;
    char *direction;
    bool optional;
    bool array;
    graph_connection_t *connections;
    size_t n_connections;
} graph_port_t;

typedef struct {
    char *block;
    uuid_t block_id;
    graph_port_t *ports;
    size_t n_ports;
} graph_block_t;

typedef struct {
    graph_block_t *blocks;
    size_t n_blocks;
} graph_t;

extern graph_t g_block_graph;

// ----------------------------------------------------------------------------

err_t graph_read_json(graph_t *graph, const cJSON *json);
err_t graph_block_read_json(graph_block_t *block, const cJSON *json);
err_t graph_port_read_json(graph_port_t *port, const cJSON *json);
err_t graph_conn_read_json(graph_connection_t *conn, const cJSON *json);

void graph_reset(graph_t *graph);
void graph_block_reset(graph_block_t *graph_block);
void graph_port_reset(graph_port_t *graph_port);
void graph_conn_reset(graph_connection_t *graph_conn);

#endif // BLOCK_GRAPH_H
