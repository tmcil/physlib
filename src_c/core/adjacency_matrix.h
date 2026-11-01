#ifndef ADJACENCY_MATRIX_H
#define ADJACENCY_MATRIX_H


#include <stdlib.h>
#include <stdbool.h>

#include "error_handling.h"


typedef struct {
    int *nodes;
    size_t n_nodes;
    int **edges;
} adjacency_matrix_t;

/* ------------------------------------------------------------------------- */

err_t adjacency_matrix_init(adjacency_matrix_t *mat,
                            const int  *nodes, const size_t n_nodes,
                            const int **edges);

/* ------------------------------------------------------------------------- */

void adjacency_matrix_free(adjacency_matrix_t *mat);

/* ------------------------------------------------------------------------- */

void adjacency_matrix_print(const adjacency_matrix_t *mat);

/* ------------------------------------------------------------------------- */

bool adjacency_matrix_is_init(const adjacency_matrix_t *mat);

/* ------------------------------------------------------------------------- */

err_t adjacency_matrix_bfs(const adjacency_matrix_t *mat, const int start_node,
                           const int end_node, int *path, int *path_len,
                           const size_t max_path_len);

/* ------------------------------------------------------------------------- */

err_t adjacency_matrix_largest_node(const adjacency_matrix_t *mat, int *largest_node);

/* ------------------------------------------------------------------------- */

err_t get_neighbours(const adjacency_matrix_t *mat, const int node,
                     int *neighbours, size_t *neighbours_len);

/* ------------------------------------------------------------------------- */

void add_to_queue(int *queue, size_t *queue_len, const int node);

/* ------------------------------------------------------------------------- */

void remove_from_queue(int *queue, size_t *queue_len, int *removed_node);

/* ------------------------------------------------------------------------- */

bool queue_is_empty(const size_t queue_len);

/* ------------------------------------------------------------------------- */

bool queue_contains(const int *queue, const size_t queue_len, const int node);

/* ------------------------------------------------------------------------- */

void add_to_explored(int *explored, int *explored_len, const int node);

/* ------------------------------------------------------------------------- */

bool explored_contains(const int *explored, const int explored_len, const int node);

/* ------------------------------------------------------------------------- */

void add_to_parents(int *parents, const int node, const int parent);

/* ------------------------------------------------------------------------- */

#endif  // ADJACENCY_MATRIX_H