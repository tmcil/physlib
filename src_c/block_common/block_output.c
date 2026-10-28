
#include "block_output.h"


/* ------------------------------------------------------------------------- */

err_t output_block_read_json(output_block_t *item, const cJSON *json) {
    // Local variables
    err_t err = NO_ERROR;    //     Status code
    size_t n_ports = 0;      //     Number of ports present in the output block
    size_t i = 0;            //     Iterator
    cJSON *block = NULL;     //     Block JSON object
    cJSON *blockId = NULL;   //     Block ID JSON object
    cJSON *ports = NULL;     //     Ports array JSON object
    cJSON *ports_el = NULL;  //     Ports array[i] JSON object
    cJSON *portId = NULL;    //     Ports array[i] ID JSON object
    cJSON *file = NULL;      //     Ports array[i] file JSON object

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

/* ------------------------------------------------------------------------- */

err_t output_block_create_json(cJSON **json, const output_block_t *item) {
    // Local variables
    cJSON *aux = NULL;           //     JSON value
    cJSON *aux_array = NULL;     //     Values array
    cJSON *aux_array_el = NULL;  //     Values array element
    size_t i = 0;                //     Iterator

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

/* ------------------------------------------------------------------------- */

err_t output_block_push(output_block_t *item, const char* port, const char* path) {
    // Local variables
    size_t i = 0;  //     Iterator

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

/* ------------------------------------------------------------------------- */

err_t output_block_init(output_block_t *item) {
    // Local variables
    err_t err = NO_ERROR;       //     Error code
    output_block_t copy = {0};  // MEM Block data being copied
    size_t i = 0, j = 0;        //     Iterators
    size_t n_output_ports = 0;  //     Number of output ports

    if (item == NULL) return ERR("Output block pointer is null");
    if (graph_isreset(g_block_graph)) return ERR("Block graph is not set");

    for (i = 0; i < g_block_graph.n_blocks; i++) {
        if (uuid_eq(g_block_graph.blocks[i].block_id, g_cmdline_args.block_id)) {
            const graph_block_t *source = &g_block_graph.blocks[i];

            copy.block = strdup(source->block);
            if (copy.block == NULL) return ERR("Failed to copy block name");

            err = uuid_set(&copy.block_id, source->block_id.val);
            if (err) {
                output_block_reset(&copy);
                return ERR("Failed to copy block ID");
            }

            for (j = 0; j < source->n_ports; j++) {
                if (strcmp(source->ports[j].direction, "output") == 0) {
                    n_output_ports++;
                }
            }

            if (n_output_ports > 0) {
                copy.ports = calloc(n_output_ports, sizeof(output_port_t));
                if (copy.ports == NULL) {
                    output_block_reset(&copy);
                    return ERR("Failed to allocate output ports");
                }
            }

            for (j = 0; j < source->n_ports; j++) {
                // skip non-output ports
                if (strcmp(source->ports[j].direction, "output") != 0) {
                    continue;
                }

                copy.ports[copy.n_ports].port_id = strdup(source->ports[j].port_id);
                if (copy.ports[copy.n_ports].port_id == NULL) {
                    output_block_reset(&copy);
                    return ERR("Failed to copy output port ID");
                }
                copy.n_ports++;
            }

            err = output_block_copy(item, &copy);
            output_block_reset(&copy);
            if (err) return ERR("Failed to copy output block");
        
            return NO_ERROR;
        }
    }

    output_block_reset(&copy);

    return ERR("Could not find this block in the block graph JSON");
}

/* ------------------------------------------------------------------------- */

err_t output_block_copy(output_block_t *dst, const output_block_t *src) {
    // Local variables
    err_t err = NO_ERROR;       //     Error code
    output_block_t copy = {0};  //     Auxiliary block, dst points here

    if (dst == NULL) return ERR("Destination output block is null");
    if (src == NULL) return ERR("Source output block is null");
    if (dst == src) return NO_ERROR;
    if (src->block == NULL || src->block_id.val == NULL) {
        return ERR("Source output block is incomplete");
    }
    if (src->n_ports > 0 && src->ports == NULL) {
        return ERR("Source output ports are missing");
    }

    copy.block = strdup(src->block);
    if (copy.block == NULL) return ERR("Failed to copy output block name");

    err = uuid_set(&copy.block_id, src->block_id.val);
    if (err) {
        output_block_reset(&copy);
        return ERR("Failed to copy output block ID");
    }

    if (src->n_ports > 0) {
        copy.ports = calloc(src->n_ports, sizeof(output_port_t));
        if (copy.ports == NULL) {
            output_block_reset(&copy);
            return ERR("Failed to allocate copied output ports");
        }
    }

    for (size_t i = 0; i < src->n_ports; i++) {
        copy.n_ports++;
        if (src->ports[i].port_id == NULL) {
            output_block_reset(&copy);
            return ERR("Source output port ID is null");
        }

        copy.ports[i].port_id = strdup(src->ports[i].port_id);
        if (copy.ports[i].port_id == NULL) {
            output_block_reset(&copy);
            return ERR("Failed to copy output port ID");
        }

        if (src->ports[i].filepath != NULL) {
            copy.ports[i].filepath = strdup(src->ports[i].filepath);
            if (copy.ports[i].filepath == NULL) {
                output_block_reset(&copy);
                return ERR("Failed to copy output file path");
            }
        }
    }

    output_block_reset(dst);
    *dst = copy;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

void output_block_reset(output_block_t *item) {
    if (item == NULL) return;
    if (item->block != NULL) free(item->block);
    item->block = NULL;
    uuid_reset(&item->block_id);
    for (size_t i = 0; i < item->n_ports; i++) {
        output_port_reset(&item->ports[i]);
    }
    free(item->ports);
    item->ports = NULL;
    item->n_ports = 0;
    return;
}

/* ------------------------------------------------------------------------- */

void output_port_reset(output_port_t *item) {
    if (item == NULL) return;
    if (item->port_id != NULL) free(item->port_id);
    if (item->filepath != NULL) free(item->filepath);
    item->port_id = NULL;
    item->filepath = NULL;
    return;
}

/* ------------------------------------------------------------------------- */
