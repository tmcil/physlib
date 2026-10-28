
#include "vector.h"


/* ------------------------------------------------------------------------- */

err_t vector_init(vector_t *vec, const int dims) {
    // Local variables
    int i = 0;  //     Iterator

    if (vec == NULL) return ERR("Vector pointer is null");
    if (dims <= 0) return ERR("Vector dimensions must be positive");
    if (vector_is_allocated(vec)) vector_free(vec);

    vec->dims = dims;
    vec->values = malloc(dims * sizeof(double));
    if (vec->values == NULL) {
        vec->dims = 0;
        return ERR("Failed to allocate vector values");
    }
    for (i = 0; i < dims; i++) {
        vec->values[i] = 0.0;
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t vector_copy(vector_t *dst, const vector_t src) {
    if (dst == NULL) return ERR("Destination vector pointer is null");
    if (!vector_is_allocated(&src)) return ERR("Source vector is not allocated");
    if (vector_is_allocated(dst) && dst->values == src.values) return ERR("Source and destination vectors alias");
    if (vector_is_allocated(dst)) vector_free(dst);

    if (vector_init(dst, src.dims)) return ERR("Failed to initialise destination vector");
    memcpy(dst->values, src.values, src.dims * sizeof(double));
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

void vector_free(vector_t *vec) {
    if (vector_is_allocated(vec)) {
        free(vec->values);
        vec->values = NULL;
        vec->dims = 0;
    }
}

/* ------------------------------------------------------------------------- */

bool vector_is_allocated(const vector_t *vec) {
    if (vec == NULL) return false;
    return (vec->values != NULL && vec->dims > 0);
}

/* ------------------------------------------------------------------------- */

err_t vector_print(char *msg, const int msg_len, const vector_t vec) {
    // Local variables
    int offset = 0;   //     Location in msg buffer to write
    int written = 0;  //     Number of characters written to msg buffer

    if (msg == NULL || msg_len <= 0) return ERR("Output buffer is invalid");
    if (!vector_is_allocated(&vec)) return ERR("Vector is not allocated");

    written = snprintf(msg + offset, msg_len - offset, "vector_t (len=%d): [", (int)vec.dims);
    if (written < 0 || written >= msg_len - offset) return ERR("Formatted vector exceeds output buffer");
    offset += written;

    for (size_t i = 0; i < vec.dims; i++) {
        const char *sep = (i == 0) ? " " : ", ";

        written = snprintf(msg + offset, msg_len - offset, "%s%.3f", sep, vec.values[i]);
        if (written < 0 || written >= msg_len - offset) return ERR("Formatted vector exceeds output buffer");
        offset += written;
    }

    written = snprintf(msg + offset, msg_len - offset, " ]");
    if (written < 0 || written >= msg_len - offset) return ERR("Formatted vector exceeds output buffer");
    offset += written;

    if (offset > msg_len) return ERR("Formatted vector exceeds output buffer");

    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

double vector_get(const vector_t vec, const int index) {
    return vec.values[index];
}

/* ------------------------------------------------------------------------- */

err_t vector_set(vector_t *vec, const int index, const double value) {
    if (!vector_is_allocated(vec)) return ERR("Vector is not allocated");
    if (index >= (int)vec->dims) return ERR("Vector index exceeds dimensions");
    if (index < 0) return ERR("Vector index cannot be negative");
    vec->values[index] = value;
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t vector_fill(vector_t *vec, const double fill_with) {
    // Local variables
    size_t i = 0;  //     Iterator

    if (!vector_is_allocated(vec)) return ERR("Vector is not allocated");
    for (i = 0; i < vec->dims; i++) {
        vec->values[i] = fill_with;
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t vector_scale(vector_t *vec, const double scale) {
    // Local variables
    size_t i = 0;  //     Iterator

    if (!vector_is_allocated(vec)) return ERR("Vector is not allocated");
    for (i = 0; i < vec->dims; i++) {
        vec->values[i] *= scale;
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t vector_add(vector_t *sum, const vector_t vec1, const vector_t vec2) {
    // Local variables
    size_t i = 0;  //     Iterator

    if (!vector_is_allocated(&vec1)) return ERR("First vector is not allocated");
    if (!vector_is_allocated(&vec2)) return ERR("Second vector is not allocated");
    if (vec1.dims != vec2.dims) return ERR("Vector dimensions do not match");

    if (vector_init(sum, vec1.dims)) return ERR("Failed to initialize sum vector");
    for (i = 0; i < vec1.dims; i++) {
        sum->values[i] = vec1.values[i] + vec2.values[i];
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t vector_sub(vector_t *vec1_minus_vec2, const vector_t vec1, const vector_t vec2) {
    // Local variables
    size_t i = 0;  //     Iterator

    if (!vector_is_allocated(&vec1)) return ERR("First vector is not allocated");
    if (!vector_is_allocated(&vec2)) return ERR("Second vector is not allocated");
    if (vec1.dims != vec2.dims) return ERR("Vector dimensions do not match");

    if (vector_init(vec1_minus_vec2, vec1.dims)) return ERR("Failed to initialize difference vector");
    for (i = 0; i < vec1.dims; i++) {
        vec1_minus_vec2->values[i] = vec1.values[i] - vec2.values[i];
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t vector_norm(double *norm, const vector_t vec) {
    // Local variables
    size_t i = 0;         //     Iterator
    double sq_sum = 0.0;  //     Sum of each element^2

    if (norm == NULL) return ERR("Norm output pointer is null");
    if (!vector_is_allocated(&vec)) return ERR("Vector is not allocated");
    for (i = 0; i < vec.dims; i++) {
        sq_sum += vec.values[i] * vec.values[i];
    }
    *norm = sqrt(sq_sum);
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t vector_unit(vector_t *unit, const vector_t vec) {
    // Local variables
    size_t i = 0;       //     Iterator
    double norm = 0.0;  //     Length of vec
    if (!vector_is_allocated(&vec)) return ERR("Vector is not allocated");
    if (unit == NULL) return ERR("Unit vector pointer is null");

    if (vector_init(unit, vec.dims)) return ERR("Failed to initialize unit vector");
    if (vector_norm(&norm, vec)) return ERR("Failed to calculate vector norm");

    for (i = 0; i < vec.dims; i++) {
        unit->values[i] = vec.values[i] / norm;
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t vector_dot(double *dot_product, const vector_t vec1, const vector_t vec2) {
    // Local variables
    size_t i = 0;  //     Iterator

    if (!vector_is_allocated(&vec1)) return ERR("First vector is not allocated");
    if (!vector_is_allocated(&vec2)) return ERR("Second vector is not allocated");
    if (dot_product == NULL) return ERR("Dot product output is null");
    if (vec1.dims != vec2.dims) return ERR("Vector dimensions do not match");

    *dot_product = 0.0;
    for (i = 0; i < vec1.dims; i ++) {
        *dot_product += vec1.values[i] * vec2.values[i];
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

err_t vector3_cross(vector_t *cross, const vector_t vec1, const vector_t vec2) {
    if (!vector_is_allocated(&vec1)) return ERR("First vector is not allocated");
    if (!vector_is_allocated(&vec2)) return ERR("Second vector is not allocated");
    if (cross == NULL) return ERR("Cross product pointer is null");
    if (vec1.dims != vec2.dims) return ERR("Vector dimensions do not match");
    if (vec1.dims != 3) return ERR("Vectors must be 3D");

    if (vector_init(cross, vec1.dims)) return ERR("Failed to initialize cross product");

    if (vector_set(cross, 0, vec1.values[1] * vec2.values[2] - vec1.values[2]*vec2.values[1])) return ERR("Failed to set cross product value");
    if (vector_set(cross, 1, vec1.values[2] * vec2.values[0] - vec1.values[0]*vec2.values[2])) return ERR("Failed to set cross product value");
    if (vector_set(cross, 2, vec1.values[0] * vec2.values[1] - vec1.values[1]*vec2.values[0])) return ERR("Failed to set cross product value");
    return NO_ERROR;
}

/* ------------------------------------------------------------------------- */

bool vector_equal(const vector_t vec1, const vector_t vec2, const double tolerance) {
    // Local variables
    bool equal = true;  //     True if vectors are equal
    size_t i = 0;       //     Iterator

    if (vec1.dims != vec2.dims) return false;
    if (!vector_is_allocated(&vec1) || !vector_is_allocated(&vec2)) return false;

    for (i = 0; i < vec1.dims; i++) {
        equal = fabs(vec1.values[i] - vec2.values[i]) > tolerance;
        if (!equal) return false;
    }
    return true;
}

/* ------------------------------------------------------------------------- */
