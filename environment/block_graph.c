#include "block_graph.h"

// Module Variables -----------------------------------------------------------

graph_t g_block_graph = {0};

// ----------------------------------------------------------------------------
err_t graph_read_json(graph_t *graph, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;
    graph_block_t block = {0};
    cJSON *blockGraph = NULL;
    cJSON *blockGraph_el = NULL;
    size_t blockGraph_len = 0;
    size_t i = 0;

    blockGraph = cJSON_GetObjectItemCaseSensitive(json, "blockGraph");
    if (!cJSON_IsArray(blockGraph)) return ERR("Failed to read: blockGraph");

    blockGraph_len = cJSON_GetArraySize(blockGraph);
    graph->n_blocks = blockGraph_len;
    graph->blocks = malloc(blockGraph_len * sizeof(graph_block_t));

    MSG("blockGraph_len: %d", (int)blockGraph_len);

    for (i = 0; i < blockGraph_len; i++) {
        blockGraph_el = cJSON_GetArrayItem(blockGraph, i);
        if (!cJSON_IsObject(blockGraph_el)) return ERR("Failed to read: blockGraph[%d]", i);
        err = graph_block_read_json(&block, blockGraph_el);
        if (err) return ERR("Failed to read: blockGraph[%d], i");
        graph->blocks[i] = block;
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t graph_block_read_json(graph_block_t *block, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;
    graph_port_t port = {0};
    cJSON *blockName = NULL;
    cJSON *blockId = NULL;
    cJSON *ports = NULL;
    cJSON *ports_el = NULL;
    size_t ports_len = 0;
    size_t i = 0;

    blockName = cJSON_GetObjectItemCaseSensitive(json, "block");
    if (!cJSON_IsString(blockName)) return ERR("Failed to read: block");

    MSG("  block: %s", blockName->valuestring);

    blockId = cJSON_GetObjectItemCaseSensitive(json, "blockId");
    if (!cJSON_IsString(blockId)) return ERR("Failed to read: blockId");

    MSG("  blockId: %s", blockId->valuestring);

    block->block = strdup(blockName->valuestring);
    err = uuid_set(&block->block_id, blockId->valuestring);
    if (err) return ERR("Failed to convert blockId to UUID: %s", blockId->valuestring);

    ports = cJSON_GetObjectItemCaseSensitive(json, "ports");
    if (!cJSON_IsArray(ports)) return ERR("Failed to read: ports");

    ports_len = cJSON_GetArraySize(ports);
    block->n_ports = ports_len;
    block->ports = malloc(ports_len * sizeof(graph_port_t));

    MSG("  ports_len: %d", (int)ports_len);

    for (i = 0; i < ports_len; i++) {
        ports_el = cJSON_GetArrayItem(ports, i);
        if (!cJSON_IsObject(ports_el)) return ERR("Failed to read: ports[%d]", i);
        err = graph_port_read_json(&port, ports_el);
        if (err) return ERR("Failed to read: ports[%d]", i);
        block->ports[i] = port;
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t graph_port_read_json(graph_port_t *port, const cJSON *json) {
    err_t err = NO_ERROR;
    graph_connection_t connection = {0};
    cJSON *portId = NULL;
    cJSON *type = NULL;
    cJSON *direction = NULL;
    cJSON *optional = NULL;
    cJSON *array = NULL;
    cJSON *connections = NULL;
    cJSON *connections_el = NULL;
    size_t connections_len = 0;
    size_t i = 0;

    portId = cJSON_GetObjectItemCaseSensitive(json, "portId");
    if (!cJSON_IsString(portId)) return ERR("Failed to read: portId");
    port->port_id = strdup(portId->valuestring);

    MSG("    portId: %s", portId->valuestring);

    type = cJSON_GetObjectItemCaseSensitive(json, "type");
    if (!cJSON_IsString(type)) return ERR("Failed to read: type");
    port->type = strdup(type->valuestring);

    MSG("    type: %s", type->valuestring);

    direction = cJSON_GetObjectItemCaseSensitive(json, "direction");
    if (!cJSON_IsString(direction)) return ERR("Failed to read: direction");
    port->direction = strdup(direction->valuestring);

    MSG("    direction: %s", direction->valuestring);

    optional = cJSON_GetObjectItemCaseSensitive(json, "optional");
    if (!cJSON_IsBool(optional)) return ERR("Failed to read: optional");
    port->optional = (bool)cJSON_IsTrue(optional);

    MSG("    optional: %d", (int)port->optional);

    array = cJSON_GetObjectItemCaseSensitive(json, "array");
    if (!cJSON_IsBool(array)) return ERR("Failed to read: array");
    port->array = (bool)cJSON_IsTrue(array);

    MSG("    array: %d", (int)port->array);

    connections = cJSON_GetObjectItemCaseSensitive(json, "connections");
    if (!cJSON_IsArray(connections)) return ERR("Failed to read: connections");

    connections_len = cJSON_GetArraySize(connections);
    port->n_connections = connections_len;
    port->connections = malloc(connections_len * sizeof(graph_connection_t));

    MSG("    connections_len: %d", (int)connections_len);

    for (i = 0; i < connections_len; i++) {
        connections_el = cJSON_GetArrayItem(connections, i);
        if (!cJSON_IsObject(connections_el)) return ERR("Failed to read: connections[%d]", i);
        err = graph_conn_read_json(&connection, connections_el);
        if (err) return ERR("Failed to read: connections[%d]", i);
        port->connections[i] = connection;
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t graph_conn_read_json(graph_connection_t *conn, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;
    cJSON *blockId = NULL;
    cJSON *portId = NULL;

    blockId = cJSON_GetObjectItemCaseSensitive(json, "blockId");
    if (!cJSON_IsString(blockId)) return ERR("Failed to read: blockId");
    err = uuid_set(&conn->block_id, blockId->valuestring);
    if (err) return ERR("Failed to convert blockId to UUID: %s", blockId->valuestring);

    MSG("      blockId: %s", blockId->valuestring);

    portId = cJSON_GetObjectItemCaseSensitive(json, "portId");
    if (!cJSON_IsString(portId)) return ERR("Failed to read: portId");
    conn->port_id = strdup(portId->valuestring);

    MSG("      portId: %s", portId->valuestring);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
void graph_reset(graph_t *graph) {
    for (size_t i = 0; i<graph->n_blocks; i++) {
        graph_block_reset(&graph->blocks[i]);
    }
    return;
}

// ----------------------------------------------------------------------------
void graph_block_reset(graph_block_t *graph_block) {
    if (graph_block->block != NULL) free(graph_block->block);
    uuid_reset(&graph_block->block_id);
    for (size_t i = 0; i<graph_block->n_ports; i++) {
        graph_port_reset(&graph_block->ports[i]);
    }
    return;
}

// ----------------------------------------------------------------------------
void graph_port_reset(graph_port_t *graph_port) {
    if (graph_port->port_id != NULL) free(graph_port->port_id);
    if (graph_port->type != NULL) free(graph_port->type);
    if (graph_port->direction != NULL) free(graph_port->direction);
    graph_port->optional = false;
    graph_port->array = false;
    for (size_t i = 0; i<graph_port->n_connections; i++) {
        graph_conn_reset(&graph_port->connections[i]);
    }
    graph_port->n_connections = 0;
    return;
}

// ----------------------------------------------------------------------------
void graph_conn_reset(graph_connection_t *graph_conn) {
    uuid_reset(&graph_conn->block_id);
    if (graph_conn->port_id != NULL) free(graph_conn->port_id);
    return;
}
