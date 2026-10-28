
#include "vector.h"


/* ------------------------------------------------------------------------- */

int vector_init(vector_t *vec, const int dims) {
    int i = 0;
    if (vec == NULL) return 1;
    if (dims <= 0) return 1;
    if (vector_is_allocated(vec)) vector_free(vec);

    vec->dims = dims;
    vec->values = malloc(dims * sizeof(double));
    if (vec->values == NULL) {
        vec->dims = 0;
        return 1;
    }
    for (i = 0; i < dims; i++) {
        vec->values[i] = 0.0;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

int vector_copy(vector_t *dst, const vector_t src) {
    if (dst == NULL) return 1;
    if (!vector_is_allocated(&src)) return 1;
    if (vector_is_allocated(dst) && dst->values == src.values) return 1;
    if (vector_is_allocated(dst)) vector_free(dst);

    if (vector_init(dst, src.dims)) return 1;
    memcpy(dst->values, src.values, src.dims * sizeof(double));
    return 0;
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

int vector_print(char *msg, const int msg_len, const vector_t vec) {
    int offset = 0;
    int written = 0;
    if (msg == NULL || msg_len <= 0) return 1;
    if (!vector_is_allocated(&vec)) return 1;

    written = snprintf(msg + offset, msg_len - offset, "vector_t (len=%d): [", (int)vec.dims);
    if (written < 0 || written >= msg_len - offset) return 1;
    offset += written;

    for (size_t i = 0; i < vec.dims; i++) {
        const char *sep = (i == 0) ? " " : ", ";

        written = snprintf(msg + offset, msg_len - offset, "%s%.3f", sep, vec.values[i]);
        if (written < 0 || written >= msg_len - offset) return 1;
        offset += written;
    }

    written = snprintf(msg + offset, msg_len - offset, " ]");
    if (written < 0 || written >= msg_len - offset) return 1;
    offset += written;

    if (offset > msg_len) return 1;

    return 0;
}

/* ------------------------------------------------------------------------- */

double vector_get(const vector_t vec, const int index) {
    return vec.values[index];
}

/* ------------------------------------------------------------------------- */

int vector_set(vector_t *vec, const int index, const double value) {
    if (!vector_is_allocated(vec)) return 1;
    if (index >= (int)vec->dims) return 1;
    if (index < 0) return 1;
    vec->values[index] = value;
    return 0;
}

/* ------------------------------------------------------------------------- */

int vector_fill(vector_t *vec, const double fill_with) {
    size_t i = 0;
    if (!vector_is_allocated(vec)) return 1;
    for (i = 0; i < vec->dims; i++) {
        vec->values[i] = fill_with;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int vector_scale(vector_t *vec, const double scale) {
    size_t i = 0;
    if (!vector_is_allocated(vec)) return 1;
    for (i = 0; i < vec->dims; i++) {
        vec->values[i] *= scale;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int vector_add(vector_t *sum, const vector_t vec1, const vector_t vec2) {
    size_t i = 0;
    if (!vector_is_allocated(&vec1)) return 1;
    if (!vector_is_allocated(&vec2)) return 1;
    if (vec1.dims != vec2.dims) return 1;

    if (vector_init(sum, vec1.dims)) return 1;
    for (i = 0; i < vec1.dims; i++) {
        sum->values[i] = vec1.values[i] + vec2.values[i];
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int vector_sub(vector_t *vec1_minus_vec2, const vector_t vec1, const vector_t vec2) {
    size_t i = 0;
    if (!vector_is_allocated(&vec1)) return 1;
    if (!vector_is_allocated(&vec2)) return 1;
    if (vec1.dims != vec2.dims) return 1;

    if (vector_init(vec1_minus_vec2, vec1.dims)) return 1;
    for (i = 0; i < vec1.dims; i++) {
        vec1_minus_vec2->values[i] = vec1.values[i] - vec2.values[i];
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int vector_norm(double *norm, const vector_t vec) {
    size_t i = 0;
    double sq_sum = 0.0;
    if (norm == NULL) return 1;
    if (!vector_is_allocated(&vec)) return 1;
    for (i = 0; i < vec.dims; i++) {
        sq_sum += vec.values[i] * vec.values[i];
    }
    *norm = sqrt(sq_sum);
    return 0;
}

/* ------------------------------------------------------------------------- */

int vector_unit(vector_t *unit, const vector_t vec) {
    size_t i = 0;
    double norm = 0.0;
    if (!vector_is_allocated(&vec)) return 1;
    if (unit == NULL) return 1;

    if (vector_init(unit, vec.dims)) return 1;
    if (vector_norm(&norm, vec)) return 1;

    for (i = 0; i < vec.dims; i++) {
        unit->values[i] = vec.values[i] / norm;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int vector_dot(double *dot_product, const vector_t vec1, const vector_t vec2) {
    size_t i = 0;
    if (!vector_is_allocated(&vec1)) return 1;
    if (!vector_is_allocated(&vec2)) return 1;
    if (dot_product == NULL) return 1;
    if (vec1.dims != vec2.dims) return 1;

    *dot_product = 0.0;
    for (i = 0; i < vec1.dims; i ++) {
        *dot_product += vec1.values[i] * vec2.values[i];
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int vector3_cross(vector_t *cross, const vector_t vec1, const vector_t vec2) {
    if (!vector_is_allocated(&vec1)) return 1;
    if (!vector_is_allocated(&vec2)) return 1;
    if (cross == NULL) return 1;
    if (vec1.dims != vec2.dims) return 1;
    if (vec1.dims != 3) return 1;

    if (vector_init(cross, vec1.dims)) return 1;

    if (vector_set(cross, 0, vec1.values[1] * vec2.values[2] - vec1.values[2]*vec2.values[1])) return 1;
    if (vector_set(cross, 1, vec1.values[2] * vec2.values[0] - vec1.values[0]*vec2.values[2])) return 1;
    if (vector_set(cross, 2, vec1.values[0] * vec2.values[1] - vec1.values[1]*vec2.values[0])) return 1;
    return 0;
}

/* ------------------------------------------------------------------------- */

bool vector_equal(const vector_t vec1, const vector_t vec2, const double tolerance) {
    bool equal = true;
    size_t i = 0;
    if (vec1.dims != vec2.dims) return false;
    if (!vector_is_allocated(&vec1) || !vector_is_allocated(&vec2)) return false;

    for (i = 0; i < vec1.dims; i++) {
        equal = fabs(vec1.values[i] - vec2.values[i]) > tolerance;
        if (!equal) return false;
    }
    return true;
}

/* ------------------------------------------------------------------------- */
