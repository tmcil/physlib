
#include "block_graph.h"

graph_t g_block_graph = {0};

/* ------------------------------------------------------------------------- */

err_t graph_read_json(graph_t *graph, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;         //     Error code
    graph_block_t block = {0};    // MEM Block data being read
    cJSON *blockGraph = NULL;     //     BlockGraph list JSON object
    cJSON *blockGraph_el = NULL;  //     BlockGraph list element object
    size_t blockGraph_len = 0;    //     Lenght of the BlockGraph list
    size_t i = 0;                 //     Iterator

    blockGraph = cJSON_GetObjectItemCaseSensitive(json, "blockGraph");
    if (!cJSON_IsArray(blockGraph)) return ERR("Failed to read: blockGraph");

    blockGraph_len = cJSON_GetArraySize(blockGraph);
    graph->n_blocks = blockGraph_len;
    graph->blocks = malloc(blockGraph_len * sizeof(graph_block_t));

    for (i = 0; i < blockGraph_len; i++) {
        blockGraph_el = cJSON_GetArrayItem(blockGraph, i);
        if (!cJSON_IsObject(blockGraph_el)) return ERR("Failed to read: blockGraph[%d]", i);
        err = graph_block_read_json(&block, blockGraph_el);
        if (err) return ERR("Failed to read: blockGraph[%d], i");
        graph->blocks[i] = block;  // TODO: deepcopy and free
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t graph_block_read_json(graph_block_t *block, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;     //     Error code
    graph_port_t port = {0};  // MEM Port data being read
    cJSON *blockName = NULL;  //     Block Name JSON object
    cJSON *blockId = NULL;    //     Block ID JSON object
    cJSON *ports = NULL;      //     Ports array JSON object
    cJSON *ports_el = NULL;   //     Ports[i] JSON object
    size_t ports_len = 0;     //     Ports array length JSON object
    size_t i = 0;             //     Iterator

    blockName = cJSON_GetObjectItemCaseSensitive(json, "block");
    if (!cJSON_IsString(blockName)) return ERR("Failed to read: block");

    blockId = cJSON_GetObjectItemCaseSensitive(json, "blockId");
    if (!cJSON_IsString(blockId)) return ERR("Failed to read: blockId");

    block->block = strdup(blockName->valuestring);
    err = uuid_set(&block->block_id, blockId->valuestring);
    if (err) return ERR("Failed to convert blockId to UUID: %s", blockId->valuestring);

    ports = cJSON_GetObjectItemCaseSensitive(json, "ports");
    if (!cJSON_IsArray(ports)) return ERR("Failed to read: ports");

    ports_len = cJSON_GetArraySize(ports);
    block->n_ports = ports_len;
    block->ports = malloc(ports_len * sizeof(graph_port_t));

    for (i = 0; i < ports_len; i++) {
        ports_el = cJSON_GetArrayItem(ports, i);
        if (!cJSON_IsObject(ports_el)) return ERR("Failed to read: ports[%d]", i);
        err = graph_port_read_json(&port, ports_el);
        if (err) return ERR("Failed to read: ports[%d]", i);
        block->ports[i] = port;  // TODO: deepcopy and free
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t graph_port_read_json(graph_port_t *port, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;                 //     Error code
    graph_connection_t connection = {0};  // MEM Connection being read
    cJSON *portId = NULL;                 //     Port ID
    cJSON *type = NULL;                   //     Product type
    cJSON *direction = NULL;              //     Port direction
    cJSON *optional = NULL;               //     Port optional flag
    cJSON *array = NULL;                  //     Port array flag
    cJSON *connections = NULL;            //     Port connections array
    cJSON *connections_el = NULL;         //     Port connections[i]
    size_t connections_len = 0;           //     Port connections array length
    size_t i = 0;                         //     Iterator

    portId = cJSON_GetObjectItemCaseSensitive(json, "portId");
    if (!cJSON_IsString(portId)) return ERR("Failed to read: portId");
    port->port_id = strdup(portId->valuestring);

    type = cJSON_GetObjectItemCaseSensitive(json, "type");
    if (!cJSON_IsString(type)) return ERR("Failed to read: type");
    port->type = strdup(type->valuestring);

    direction = cJSON_GetObjectItemCaseSensitive(json, "direction");
    if (!cJSON_IsString(direction)) return ERR("Failed to read: direction");
    port->direction = strdup(direction->valuestring);

    optional = cJSON_GetObjectItemCaseSensitive(json, "optional");
    if (!cJSON_IsBool(optional)) return ERR("Failed to read: optional");
    port->optional = (bool)cJSON_IsTrue(optional);

    array = cJSON_GetObjectItemCaseSensitive(json, "array");
    if (!cJSON_IsBool(array)) return ERR("Failed to read: array");
    port->array = (bool)cJSON_IsTrue(array);

    connections = cJSON_GetObjectItemCaseSensitive(json, "connections");
    if (!cJSON_IsArray(connections)) return ERR("Failed to read: connections");

    connections_len = cJSON_GetArraySize(connections);
    port->n_connections = connections_len;
    port->connections = malloc(connections_len * sizeof(graph_connection_t));

    for (i = 0; i < connections_len; i++) {
        connections_el = cJSON_GetArrayItem(connections, i);
        if (!cJSON_IsObject(connections_el)) return ERR("Failed to read: connections[%d]", i);
        err = graph_conn_read_json(&connection, connections_el);
        if (err) return ERR("Failed to read: connections[%d]", i);
        port->connections[i] = connection; // todo deepcopy and free
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t graph_conn_read_json(graph_connection_t *conn, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;   //     Error code
    cJSON *blockId = NULL;  //     Block ID
    cJSON *portId = NULL;   //     Port ID

    blockId = cJSON_GetObjectItemCaseSensitive(json, "blockId");
    if (!cJSON_IsString(blockId)) return ERR("Failed to read: blockId");
    err = uuid_set(&conn->block_id, blockId->valuestring);
    if (err) return ERR("Failed to convert blockId to UUID: %s", blockId->valuestring);

    portId = cJSON_GetObjectItemCaseSensitive(json, "portId");
    if (!cJSON_IsString(portId)) return ERR("Failed to read: portId");
    conn->port_id = strdup(portId->valuestring);

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t graph_find_input_port_file(cJSON **j_target, const uuid_t this_block_id,
                                 const char* port) {
    // Local variables
    err_t err = NO_ERROR;                 //     Error code
    char output_file_buf[512];            //     Buffer to hold name of file
    graph_block_t this_block = {0};       //     The current block
    graph_port_t this_port = {0};         //     Port of the current block
    graph_connection_t connection = {0};  //     Input connection
    graph_block_t target_block = {0};     //     Target block
    cJSON *j_output_target = NULL;        // MEM Output JSON tree
    output_block_t output_target = {0};   // MEM Output block data
    size_t i = 0;
    bool found = false;
    int stat = 0;

    if (graph_isreset(g_block_graph)) return ERR("Block graph object is not set");
    if (port == NULL) return ERR("NULL port pointer passed to function");

    err = graph_search_for_block(&this_block, this_block_id);
    if (err) return ERR("Failed to find the current block in the block list");

    err = graph_block_search_for_port(&this_port, this_block, port, "input");
    if (err) return ERR("Failed to find the requested port in the current block");

    // Input ports only every have one connection. So we can read directly.
    connection = this_port.connections[0];

    // Now the target block and port is known, we can read the output file
    // generated by that program.
    err = graph_search_for_block(&target_block, connection.block_id);
    if (err) return ERR("Failed to find block connected to port [%s]", this_port.port_id);

    stat = snprintf(output_file_buf, 512, "%s_%s.json", target_block.block,
                    uuid_tostring_short(target_block.block_id));
    if (stat < 0) return ERR("Failed to construct filename");
    if ((size_t)stat >= 512) return ERR("Filename too long");

    // Read the target file and parse the JSON
    err = working_dir_read(&j_output_target, OUTPUTS, output_file_buf);
    if (err) return ERR("Failed to read input port program output file: %s", output_file_buf);

    // The file to return is the one on connection.portId
    err = output_block_read_json(&output_target, j_output_target);
    if (err) return ERR("Failed to create target object");

    for (i = 0; i < output_target.n_ports; i++) {
        if (strcmp(output_target.ports[i].port_id, connection.port_id) == 0) {
            found = true;
            err = working_dir_read(j_target, FILES, output_target.ports[i].filepath);
            if (err) return ERR("Failed to read JSON file: %s", output_target.ports[i].filepath);
        }
    }
    if (!found) return ERR("Could not find requested port in output of connected program");

    cJSON_Delete(j_output_target);
    output_block_reset(&output_target);

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t graph_block_search_for_port(graph_port_t *port, const graph_block_t block,
                                  const char* port_id, const char* direction) {
    // Local variables
    size_t i = 0;        //     Iterator
    bool found = false;  //     True if port is found

    // Search this blocks ports for the requested one.
    for (i = 0; i < block.n_ports; i++) {
        if (strcmp(block.ports[i].port_id,
                   port_id) == 0) {
            found = true;
            *port = block.ports[i];
            break;
       }
    }
    if (!found) return ERR("Could not find port [%s] in the block", port_id);

    if (strcmp(port->direction, direction) != 0) {
        return ERR("Requested port [%s] does not have the reqested direction [%s]", port_id, direction);
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t graph_search_for_block(graph_block_t *block, const uuid_t id) {
    // Local variables
    size_t i = 0;        //     Iterator
    bool found = false;  //     True of block is found

    // Search for this block in the block list.
    for (i = 0; i < g_block_graph.n_blocks; i++) {
        if (strcmp(uuid_tostring(g_block_graph.blocks[i].block_id),
                   uuid_tostring(id)) == 0) {
            *block = g_block_graph.blocks[i];
            found = true;
            break;
       } 
    }
    if (!found) return ERR("Could not find block [%s] in the block list",
                           uuid_tostring(id));

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

bool graph_isreset(const graph_t graph) {
    return (graph.blocks == NULL && graph.n_blocks == 0);
}

/* ------------------------------------------------------------------------- */

void graph_reset(graph_t *graph) {
    for (size_t i = 0; i<graph->n_blocks; i++) {
        graph_block_reset(&graph->blocks[i]);
    }
    graph->n_blocks = 0;
    return;
}

/* ------------------------------------------------------------------------- */

void graph_block_reset(graph_block_t *graph_block) {
    if (graph_block->block != NULL) free(graph_block->block);
    uuid_reset(&graph_block->block_id);
    for (size_t i = 0; i<graph_block->n_ports; i++) {
        graph_port_reset(&graph_block->ports[i]);
    }
    graph_block->n_ports = 0;
    return;
}

/* ------------------------------------------------------------------------- */

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

/* ------------------------------------------------------------------------- */

void graph_conn_reset(graph_connection_t *graph_conn) {
    uuid_reset(&graph_conn->block_id);
    if (graph_conn->port_id != NULL) free(graph_conn->port_id);
    return;
}

/* ------------------------------------------------------------------------- */
