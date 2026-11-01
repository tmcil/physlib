
#include "adjacency_matrix.h"


/* ------------------------------------------------------------------------- */

err_t adjacency_matrix_init(adjacency_matrix_t *mat,
                            const int  *nodes, const size_t n_nodes,
                            const int **edges) {
    // Local variables
    size_t i = 0;  //     Iterator
    size_t j = 0;  //     Iterator

    if (mat == NULL) return ERR("NULL adjacency matrix argument");
    if (nodes == NULL) return ERR("Invalid nodes definition");
    if (edges == NULL) return ERR("Invalid edges definition");

    mat->n_nodes = 0;
    mat->nodes = NULL;
    mat->edges = NULL;

    // Check that 0 is not present in the node list - it is reserved for
    // connection
    for (i = 0; i < n_nodes; i++) {
        if (nodes[i] == 0) return ERR("Nodes cannot have index 0. It is reserved for `no connection`");
    }

    // Allocate the nodes
    mat->nodes = malloc(n_nodes * sizeof(int));
    if (mat->nodes == NULL) return ERR("Failed to allocate nodes memory");
    for (i = 0; i < n_nodes; i++) {
        mat->nodes[i] = nodes[i];
    }

    // Allocate the edges. There must be edges defined for every node, and
    // each edge has connection info to each node. Therefor edges is
    // n_nodes x n_nodes, by definition.
    mat->edges = malloc(n_nodes * sizeof(int*));
    if (mat->edges == NULL) return ERR("Failed to allocate edge list memory");
    for (i = 0; i < n_nodes; i++) {
        mat->edges[i] = malloc(n_nodes * sizeof(int));
        if (mat->edges[i] == NULL) return ERR("Failed to allocate edge[%d] memory", (int)i);
    }
    for (i = 0; i < n_nodes; i++) {  // rows
        for (j = 0; j < n_nodes; j++) {  // cols
            mat->edges[i][j] = edges[i][j];
        }
    }

    mat->n_nodes = n_nodes;

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

void adjacency_matrix_free(adjacency_matrix_t *mat) {
    // Local variables
    size_t i = 0;  //     Iterator

    if (mat == NULL) return;
    if (mat->n_nodes == 0) return;

    if (mat->nodes != NULL) {
        free(mat->nodes);
        mat->nodes = NULL;
    }

    if (mat->edges != NULL) {
        for (i = 0; i < mat->n_nodes; i++){
            if (mat->edges[i] != NULL){
                free(mat->edges[i]);
                mat->edges[i] = NULL;
            }
        }

        free(mat->edges);
        mat->edges = NULL;
    }

    mat->n_nodes = 0;

    return;
}

/* ------------------------------------------------------------------------- */

void adjacency_matrix_print(const adjacency_matrix_t *mat) {
    // Local variables
    bool failed = false;
    size_t i = 0;       //     Iterator
    size_t j = 0;       //     Iterator
    char buf[256];      //     Message buffer
    size_t offset = 0;  //     Offset into the buffer

    if (mat == NULL) failed = true;
    if (mat->n_nodes <= 0 || mat->nodes == NULL) failed = true;
    if (mat->edges == NULL) failed = true;

    if (failed) {
        MSG("Attempted to print adjacency matrix but it did not pass sanity checks");
        return;
    } else {
        MSG("Printing adjacency matrix with %zu nodes:", mat->n_nodes);
    }

    // Print a header row, so that the connected nodes is obvious
    for (i = 0; i < mat->n_nodes; i++) {
        offset += snprintf(buf + offset, 256 - offset, "%02d ", mat->nodes[i]);
    }
    MSG("nodes    | %s", buf);
    offset = 0;
    for (i = 0; i < mat-> n_nodes; i++) {
        offset += snprintf(buf + offset, 256 - offset, "---");
    }
    MSG("---------+%s", buf);

    for (i = 0; i < mat->n_nodes; i++) {
        // Reset the offset
        offset = 0;

        for (j = 0; j < mat->n_nodes; j++) {
            offset += snprintf(buf + offset, 256 - offset, "%s ", mat->edges[i][j] ? "XX": "  ");
        }

        // Print the row
        MSG("node (%02d)| %s", mat->nodes[i], buf);
    }

    MSG("Printing adjacency matrix finished");

    return;
}

/* ------------------------------------------------------------------------- */

bool adjacency_matrix_is_init(const adjacency_matrix_t *mat) {
    if (mat == NULL) return false;
    if (mat->n_nodes <= 0 || mat->nodes == NULL) return false;
    if (mat->edges == NULL) return false;
    return true;
}

/* ------------------------------------------------------------------------- */

err_t adjacency_matrix_bfs(const adjacency_matrix_t *mat, const int start_node,
                           const int end_node, int *path, int *path_len, 
                           const size_t max_path_len) {
    // Local variables
    err_t status = NO_ERROR;   //     Error code
    bool failed = false;       //     Flag for failure
    size_t i = 0;              //     Iterator
    bool found_start = false;  //     Flag for start node found
    bool found_end = false;    //     Flag for end node found
    int current_node = 0;      //     Current node being processed
    int largest_node = 0;      //     Largest node value in the adjacency matrix
    int *queue = NULL;         // MEM Queue for BFS
    size_t queue_len = 0;      //     Length of the queue
    int *neighbours = NULL;    // MEM Neighbours of the current node
    size_t neighbours_len = 0; //     Number of neighbours of the current node
    int *explored = NULL;      // MEM Explored path
    int explored_len = 0;      //     Length of the explored path
    int *solution = NULL;      // MEM Solution path
    size_t solution_len = 0;   //     Length of the solution path
    int *parents = NULL;       // MEM Parents of the nodes

    if (mat == NULL) return ERR("NULL adjacency matrix argument");    
    if (path == NULL) return ERR("NULL path argument");
    if (!adjacency_matrix_is_init(mat)) return ERR("Adjacency matrix is not initialized");
    for (i = 0; i < mat->n_nodes; i++) {
        if (mat->nodes[i] == start_node) found_start = true;
        if (mat->nodes[i] == end_node) found_end = true;
    }
    if (!found_start) return ERR("Start node not found: %d", start_node);
    if (!found_end)   return ERR("End node not found: %d", end_node);

    // Initilise the queue, max length is the number of nodes
    queue = calloc((mat->n_nodes), sizeof(int));
    if (queue == NULL) return ERR("Failed to allocate BFS queue memory");

    // Initialise the neighbours array, max length is the number of nodes
    neighbours = calloc((mat->n_nodes), sizeof(int));
    if (neighbours == NULL) return ERR("Failed to allocate BFS neighbours memory");

    // Initialise the explored array, max length is the number of nodes
    explored = calloc((mat->n_nodes), sizeof(int));
    if (explored == NULL) return ERR("Failed to allocate BFS explored memory");

    // Initialise the solution path, max length is max_path_len
    solution = calloc(max_path_len, sizeof(int));
    if (solution == NULL) return ERR("Failed to allocate BFS solution path memory");

    // Initialise the parents array, length of array is value of largest node + 1
    // This simplifies reconstructing the path, at the code of slightly more memory alloc.
    status = adjacency_matrix_largest_node(mat, &largest_node);
    if (status != NO_ERROR) return ERR("Failed to get largest node in adjacency matrix");
    parents = calloc((largest_node + 1), sizeof(int));
    if (parents == NULL) return ERR("Failed to allocate BFS parents memory");

    // Start the BFS algorithm
    queue[0] = start_node;
    queue_len = 1;
    while (!queue_is_empty(queue_len)) {

        // Explore the current node
        remove_from_queue(queue, &queue_len, &current_node);
        add_to_explored(explored, &explored_len, current_node);

        // Check if the current node is the end node
        if (current_node == end_node) {
            break;
        }

        // Get the neighbours of the current node
        status = get_neighbours(mat, current_node, neighbours, &neighbours_len);
        if (status != NO_ERROR) {
            ERR("Failed to get neighbours of node %d", current_node);
            failed = true;
            break;
        }

        // Add the neighbours to the queue if they have not been explored or 
        // are not already in the queue
        for (i = 0; i < neighbours_len; i++) {
            if (!explored_contains(explored, explored_len, neighbours[i]) &&
                !queue_contains(queue, queue_len, neighbours[i])) {
                add_to_queue(queue, &queue_len, neighbours[i]);
                add_to_parents(parents, neighbours[i], current_node);
            }
        }
    }

    // Reconstruct the path. First construct the path in reverse order, then
    // reverse it to retrieve the solution
    if (!failed) {
        solution[0] = current_node;
        solution_len++;

        // Reconstruct the path
        while (current_node != start_node) {
            // Get the parent of the current node
            current_node = parents[current_node];
            solution[solution_len] = current_node;
            solution_len++;
        }

        // Reverse the solution path
        for (i = 0; i < solution_len; i++) {
            path[i] = solution[solution_len - 1 - i];
        }

        // Set the path length
        *path_len = solution_len;
    }

    // Free memory
    if (solution != NULL) free(solution);
    if (queue != NULL) free(queue);
    if (parents != NULL) free(parents);
    if (explored != NULL) free(explored);
    if (neighbours != NULL) free(neighbours);

    if (failed) return ERR("BFS failed");
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t adjacency_matrix_largest_node(const adjacency_matrix_t *mat, int *largest_node) {
    // Local variables
    size_t i = 0;          //     Iterator
    int temp_largest = 0;  //     Temporary largest node

    if (mat == NULL) return ERR("NULL adjacency matrix argument");
    if (largest_node == NULL) return ERR("NULL largest node argument");

    if (!adjacency_matrix_is_init(mat)) return ERR("Adjacency matrix is not initialized");

    for (i = 0; i < mat->n_nodes; i++) {
        if (mat->nodes[i] > temp_largest) temp_largest = mat->nodes[i];
    }

    *largest_node = temp_largest;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t get_neighbours(const adjacency_matrix_t *mat, const int node,
                     int *neighbours, size_t *neighbours_len) {
    // Local variables
    size_t i = 0;        //     Iterator
    size_t j = 0;        //     Iterator
    bool found = false;  //     Flag for node found in the adjacency matrix

    if (mat == NULL) return ERR("NULL adjacency matrix argument");
    if (!adjacency_matrix_is_init(mat)) return ERR("Adjacency matrix is not initialized");
    if (neighbours == NULL) return ERR("NULL neighbours array argument");
    if (neighbours_len == NULL) return ERR("NULL number of neighbours argument");

    // Find the node in the matrix
    for (i = 0; i < mat->n_nodes; i++) {
        if (mat->nodes[i] == node) {
            found = true;
            break;
        }
    }
    if (!found) return ERR("Node %d not found in adjacency matrix", node);

    // Get the neighbours of the node
    *neighbours_len = 0;
    for (j = 0; j < mat->n_nodes; j++) {
        if (mat->edges[i][j] != 0) {
            neighbours[*neighbours_len] = mat->nodes[j];
            (*neighbours_len)++;
        }
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

void add_to_queue(int *queue, size_t *queue_len, const int node) {
    queue[*queue_len] = node;
    (*queue_len)++;
    return;
}

/* ------------------------------------------------------------------------- */

void remove_from_queue(int *queue, size_t *queue_len, int *removed_node) {
    if (*queue_len == 0) return;
    *removed_node = queue[0];
    for (size_t i = 0; i < (*queue_len - 1); i++) {
        queue[i] = queue[i + 1];
    }
    (*queue_len)--;
    return;
}

/* ------------------------------------------------------------------------- */

bool queue_is_empty(const size_t queue_len) {
    return (queue_len == 0);
}

/* ------------------------------------------------------------------------- */

bool queue_contains(const int *queue, const size_t queue_len, const int node) {
    for (size_t i = 0; i < queue_len; i++) {
        if (queue[i] == node) return true;
    }
    return false;
}

/* ------------------------------------------------------------------------- */

void add_to_explored(int *explored, int *explored_len, const int node) {
    explored[*explored_len] = node;
    (*explored_len)++;
    return;
}

/* ------------------------------------------------------------------------- */

bool explored_contains(const int *explored, const int explored_len, const int node) {
    for (int i = 0; i < explored_len; i++) {
        if (explored[i] == node) return true;
    }
    return false;
}

/* ------------------------------------------------------------------------- */

void add_to_parents(int *parents, const int node, const int parent) {
    // Only add the parent if it has not already been set.
    if (parents[node] == 0) {
        parents[node] = parent;
    }
    return;
}

/* ------------------------------------------------------------------------- */
