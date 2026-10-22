#include "block_output.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t output_block_read_json(output_block_t *item, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;
    cJSON *block = NULL;
    cJSON *blockId = NULL;
    cJSON *ports = NULL;
    cJSON *ports_el = NULL;
    size_t n_ports = 0;
    cJSON *portId = NULL;
    cJSON *file = NULL;
    size_t i = 0;

    block = cJSON_GetObjectItemCaseSensitive(json, "block");
    if (!cJSON_IsString(block)) return ERR("Failed to read JSON: block");

    blockId = cJSON_GetObjectItemCaseSensitive(json, "blockId");
    if (!cJSON_IsString(blockId)) return ERR("Failed to read JSON: blockId");

    item->block = strdup(block->valuestring);
    err = uuid_set(&item->block_id, blockId->valuestring);
    if (err) return ERR("Failed to convert ID [%s] to UUID", blockId->valuestring);

    ports = cJSON_GetObjectItemCaseSensitive(json, "ports");
    if (!cJSON_IsArray(ports)) return ERR("Failed to read JSON: ports");
    n_ports = cJSON_GetArraySize(ports);

    item->ports = malloc(n_ports * sizeof(output_port_t));

    for (i = 0; i < n_ports; i++) {
        ports_el = cJSON_GetArrayItem(ports, i);
        if (!cJSON_IsObject(ports_el)) return ERR("Failed to read JSON: ports[%s]", i );

        portId = cJSON_GetObjectItemCaseSensitive(ports_el, "portId");
        if (!cJSON_IsString(portId)) return ERR("Failed to read JSON: ports[%s].portId", i);

        file = cJSON_GetObjectItemCaseSensitive(ports_el, "file");
        if (!cJSON_IsString(file)) return ERR("Failed to read JSON: ports[%s].file", i);

        item->ports[i].port_id = strdup(portId->valuestring);
        item->ports[i].filepath = strdup(file->valuestring);
    }
    
    item->n_ports = n_ports;

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t output_block_create_json(cJSON **json, const output_block_t *item) {
    // Local variables
    cJSON *aux = NULL;
    cJSON *aux_array = NULL;
    cJSON *aux_array_el = NULL;
    size_t i = 0;

    if (item == NULL) return ERR("Null item pointer passed to function");

    *json = cJSON_CreateObject();
    if (*json == NULL) return ERR("Failed to create JSON object");

    aux = cJSON_CreateString(item->block);
    if (aux==NULL) return ERR("Failed to create JSON: block");
    cJSON_AddItemToObject(*json, "block", aux);

    aux = cJSON_CreateString(uuid_tostring(item->block_id));
    if (aux==NULL) return ERR("Failed to create JSON: block_id");
    cJSON_AddItemToObject(*json, "blockId", aux);

    aux_array = cJSON_AddArrayToObject(*json, "ports");
    if (aux_array==NULL) return ERR("Failed to create JSON: ports");

    for (i = 0; i < item->n_ports; i++) {

        aux_array_el = cJSON_CreateObject();
        if (aux_array_el == NULL) return ERR("Failed to create object for array");
        cJSON_AddItemToArray(aux_array, aux_array_el);

        aux = cJSON_CreateString(item->ports[i].port_id);
        if (!cJSON_IsString(aux)) return ERR("Failed to create JSON: ports[%d].portId", i);
        cJSON_AddItemToObject(aux_array_el, "portId", aux);

        if (item->ports[i].filepath == NULL) {
            WAR("Port [%s] is not connected to any other port. Its product will be discarded.", item->ports[i].port_id);
            aux = cJSON_CreateNull();
            cJSON_AddNullToObject(aux_array_el, "file");
        } else {
            aux = cJSON_CreateString(item->ports[i].filepath);
            if (!cJSON_IsString(aux)) return ERR("Failed to create JSON: ports[%d].filepath", i);
            cJSON_AddItemToObject(aux_array_el, "file", aux);
        }
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t output_block_push(output_block_t *item, const char* port, const char* path) {
    // Local variables
    size_t i = 0;

    if (item == NULL) return ERR("NULL item pointer passed to function");
    if (port == NULL) return ERR("NULL port pointer passed to function");
    if (path == NULL) return ERR("NULL path pointer passed to function");

    for (i = 0; i < item->n_ports; i++) {
        if (strcmp(item->ports[i].port_id, port) == 0) {

            item->ports[i].filepath = strdup(path);
            return NO_ERROR;
        }
    }

    return ERR("Failed to find port [%s] in output block type");
}

// ----------------------------------------------------------------------------
err_t output_block_init(output_block_t *item) {
    // Local variables
    size_t i = 0, j = 0;
    size_t n_output_ports = 0;

    if (graph_isreset(g_block_graph)) return ERR("Block graph is not set");

    for (i = 0; i < g_block_graph.n_blocks; i++) {
        if (uuid_eq(g_block_graph.blocks[i].block_id, g_cmdline_args.block_id)) {
            item->block = g_block_graph.blocks[i].block;
            item->block_id = g_block_graph.blocks[i].block_id;

            for (j = 0; j < g_block_graph.blocks[i].n_ports; j++) {
                // skip non-output ports
                if (strcmp(g_block_graph.blocks[i].ports[j].direction, "output") != 0) {
                    continue;
                }

                n_output_ports++;
                item->ports = realloc(item->ports, n_output_ports * sizeof(output_port_t));
                item->ports[n_output_ports-1].filepath = NULL;
                item->ports[n_output_ports-1].port_id = g_block_graph.blocks[i].ports[j].port_id;
            }
            item->n_ports = n_output_ports;
        
            return NO_ERROR;
        }
    }

    return ERR("Could not find this block in the block graph JSON");
}

// ----------------------------------------------------------------------------
void output_block_reset(output_block_t *item) {
    if (item->block != NULL) free(item->block);
    uuid_reset(&item->block_id);
    for (size_t i = 0; i < item->n_ports; i++) {
        output_port_reset(&item->ports[i]);
    }
    item->n_ports = 0;
    return;
}

// ----------------------------------------------------------------------------
void output_port_reset(output_port_t *item) {
    if (item->port_id != NULL) free(item->port_id);
    if (item->filepath != NULL) free(item->filepath);
    return;
}
