
#include "matrix.h"


/* ------------------------------------------------------------------------- */

int matrix_init(matrix_t *mat, const int rows, const int cols) {
    if (mat == NULL) return 1;
    if (rows <= 0 || cols <= 0) return 1;
    if (matrix_is_allocated(mat)) matrix_free(mat);

    mat->rows = rows;
    mat->cols = cols;
    mat->values = malloc(rows * cols* sizeof(double));
    if (mat->values == NULL) {
        mat->rows = 0;
        mat->cols = 0;
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int matrix_init_identity(matrix_t *mat, const int dims) {
    int status = 0;
    int i = 0;
    int j = 0;
    if (matrix_init(mat, dims, dims) != 0) return 1;
    for (i = 0; i < dims; i++) {
        for (j = 0; j < dims; j++) {
            if (i == j) {
                if (matrix_set(mat, i, j, 1.0)) status = 1;
            } else {
                if (matrix_set(mat, i, j, 0.0)) status = 1;
            }
        }
    }
    if (status == 1) {
        matrix_free(mat);
        return 1;
    } else{
        return 0;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int matrix_copy(matrix_t *dst, const matrix_t src) {
    if (dst == NULL) return 1;
    if (!matrix_is_allocated(&src)) return 1;
    if (matrix_is_allocated(dst) && dst->values == src.values) return 1;
    if (matrix_is_allocated(dst)) matrix_free(dst);

    if (matrix_init(dst, src.rows, src.cols)) return 1;
    memcpy(dst->values, src.values, src.rows * src.cols * sizeof(double));
    return 0;
}

/* ------------------------------------------------------------------------- */

void matrix_free(matrix_t *mat) {
    if (matrix_is_allocated(mat)) {
        free(mat->values);
        mat->values = NULL;
        mat->rows = 0;
        mat->cols = 0;
    }
}

/* ------------------------------------------------------------------------- */

bool matrix_is_allocated(const matrix_t *mat) {
    if (mat == NULL) return false;
    return (mat->values != NULL && mat->rows > 0 && mat->cols > 0);
}

/* ------------------------------------------------------------------------- */

bool matrix_is_square(const matrix_t mat) {
    if (!matrix_is_allocated(&mat)) return false;
    return mat.cols == mat.rows;
}

/* ------------------------------------------------------------------------- */

int matrix_print(char *msg, const int msg_len, const matrix_t mat) {
    int offset = 0;
    int written = 0;

    if (msg == NULL || msg_len <= 0) return 1;
    if (!matrix_is_allocated(&mat)) return 1;

    written = snprintf(msg + offset, msg_len - offset,
                       "matrix_t (rows=%d, cols=%d): [",
                       (int)mat.rows, (int)mat.cols);
    if (written < 0 || written >= msg_len - offset) return 1;
    offset += written;

    for (size_t row = 0; row < mat.rows; row++) {
        const char *row_sep = (row == 0) ? " " : ", ";

        written = snprintf(msg + offset, msg_len - offset,
                           "%s[", row_sep);
        if (written < 0 || written >= msg_len - offset) return 1;
        offset += written;

        for (size_t col = 0; col < mat.cols; col++) {
            const char *sep = (col == 0) ? " " : ", ";

            written = snprintf(msg + offset, msg_len - offset,
                               "%s%.3f", sep, matrix_get(mat, row, col));
            if (written < 0 || written >= msg_len - offset) return 1;
            offset += written;
        }

        written = snprintf(msg + offset, msg_len - offset, " ]");
        if (written < 0 || written >= msg_len - offset) return 1;
        offset += written;
    }

    written = snprintf(msg + offset, msg_len - offset, "\n]");
    if (written < 0 || written >= msg_len - offset) return 1;

    return 0;
}

/* ------------------------------------------------------------------------- */

double matrix_get(const matrix_t mat, const int row, const int col) {
    return mat.values[(row*mat.cols) + col];
}

/* ------------------------------------------------------------------------- */

int matrix_set(matrix_t *mat, const int row, const int col, const double value) {
    if (!matrix_is_allocated(mat)) return 1;
    if (row >= (int)mat->rows || col >= (int)mat->cols) return 1;
    mat->values[(row*mat->cols) + col] = value;
    return 0;
}

/* ------------------------------------------------------------------------- */

int matrix_fill(matrix_t *mat, const double fill_with) {
    size_t i = 0;
    size_t j = 0;
    if (!matrix_is_allocated(mat)) return 1;
    for (i = 0; i < mat->rows; i++) {
        for (j = 0; j < mat->cols; j++) {
            if (matrix_set(mat, i, j, fill_with) != 0) return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int matrix_scale(matrix_t *mat, const double scale) {
    size_t i = 0;
    size_t j = 0;
    if (!matrix_is_allocated(mat)) return 1;
    for (i = 0; i < mat->rows; i++) {
        for (j = 0; j < mat->cols; j++) {
            if (matrix_set(mat, i, j, scale * matrix_get(*mat, i, j)) != 0) return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int matrix_add(matrix_t *sum, const matrix_t mat1, const matrix_t mat2) {
    int status = 0;
    size_t i = 0;
    size_t j = 0;
    if (!matrix_is_allocated(&mat1)) return 1;
    if (!matrix_is_allocated(&mat2)) return 1;
    if (mat1.rows != mat2.rows || mat1.cols != mat2.cols) return 1;

    if (matrix_init(sum, mat1.rows, mat1.cols)) return 1;
    for (i = 0; i < mat1.rows; i++) {
        for (j = 0; j < mat1.cols; j++) {
            if (matrix_set(sum, i, j, matrix_get(mat1, i, j) + matrix_get(mat2, i, j))) status = 1;
        }
    }
    if (status == 1) {
        matrix_free(sum);
        return 1;
    } else {
        return 0;
    }
}

/* ------------------------------------------------------------------------- */

int matris_sub(matrix_t *mat1_minus_mat2, const matrix_t mat1, const matrix_t mat2) {
    int status = 0;
    size_t i = 0;
    size_t j = 0;
    if (!matrix_is_allocated(&mat1)) return 1;
    if (!matrix_is_allocated(&mat2)) return 1;
    if (mat1.rows != mat2.rows || mat1.cols != mat2.cols) return 1;

    if (matrix_init(mat1_minus_mat2, mat1.rows, mat1.cols)) return 1;
    for (i = 0; i < mat1.rows; i++) {
        for (j = 0; j < mat1.cols; j++) {
            if (matrix_set(mat1_minus_mat2, i, j, matrix_get(mat1, i, j) - matrix_get(mat2, i, j))) status = 1;
        }
    }
    if (status == 1) {
        matrix_free(mat1_minus_mat2);
        return 1;
    } else {
        return 0;
    }
}

/* ------------------------------------------------------------------------- */

int matrix_transpose(matrix_t *mat_T, const matrix_t mat) {
    int status = 0;
    size_t i = 0;
    size_t j = 0;
    if (!matrix_is_allocated(&mat)) return 1;

    if (matrix_init(mat_T, mat.cols, mat.rows)) return 1;
    for (i = 0; i < mat.rows; i++) {
        for (j = 0; j < mat.cols; j++) {
            if (matrix_set(mat_T, j, i, matrix_get(mat, i, j))) status = 1;
        }
    }
    if (status == 1) {
        matrix_free(mat_T);
        return 1;
    } else {
        return 0;
    }
}

/* ------------------------------------------------------------------------- */

int matrix2_mul(matrix_t *axb, const matrix_t a, const matrix_t b) {
    int status = 0;
    if (!matrix_is_allocated(&a)) return 1;
    if (!matrix_is_allocated(&b)) return 1;
    if (a.cols != 2 || b.cols != 2 || a.rows != 2 || b.rows != 2) return 1;
    if (matrix_is_allocated(axb)) matrix_free(axb);

    if (matrix_init(axb, 2, 2)) return 1;
    if (matrix_set(axb, 0, 0, matrix_get(a, 0, 0)*matrix_get(b, 0, 0) + matrix_get(a, 0, 1)*matrix_get(b, 1, 0))) status = 1;
    if (matrix_set(axb, 0, 1, matrix_get(a, 0, 0)*matrix_get(b, 0, 1) + matrix_get(a, 0, 1)*matrix_get(b, 1, 1))) status = 1;
    if (matrix_set(axb, 1, 0, matrix_get(a, 1, 0)*matrix_get(b, 0, 0) + matrix_get(a, 1, 1)*matrix_get(b, 1, 0))) status = 1;
    if (matrix_set(axb, 1, 1, matrix_get(a, 1, 0)*matrix_get(b, 0, 1) + matrix_get(a, 1, 1)*matrix_get(b, 1, 1))) status = 1;
    
    if (status == 1) {
        matrix_free(axb);
        return 1;
    } else {
        return 0;
    }
}

/* ------------------------------------------------------------------------- */

int matrix3_mul(matrix_t *axb, const matrix_t a, const matrix_t b) {
    int status = 0;
    if (!matrix_is_allocated(&a)) return 1;
    if (!matrix_is_allocated(&b)) return 1;
    if (a.cols != 3 || b.cols != 3 || a.rows != 3 || b.rows != 3) return 1;
    if (matrix_is_allocated(axb)) matrix_free(axb);

    if (matrix_init(axb, 3, 3)) return 1;

    if (matrix_set(axb, 0, 0, matrix_get(a, 0, 0)*matrix_get(b, 0, 0) + matrix_get(a, 0, 1)*matrix_get(b, 1, 0) + matrix_get(a, 0, 2)*matrix_get(b, 2, 0))) status = 1;
    if (matrix_set(axb, 0, 1, matrix_get(a, 0, 0)*matrix_get(b, 0, 1) + matrix_get(a, 0, 1)*matrix_get(b, 1, 1) + matrix_get(a, 0, 2)*matrix_get(b, 2, 1))) status = 1;
    if (matrix_set(axb, 0, 2, matrix_get(a, 0, 0)*matrix_get(b, 0, 2) + matrix_get(a, 0, 1)*matrix_get(b, 1, 2) + matrix_get(a, 0, 2)*matrix_get(b, 2, 2))) status = 1;
    if (matrix_set(axb, 1, 0, matrix_get(a, 1, 0)*matrix_get(b, 0, 0) + matrix_get(a, 1, 1)*matrix_get(b, 1, 0) + matrix_get(a, 1, 2)*matrix_get(b, 2, 0))) status = 1;
    if (matrix_set(axb, 1, 1, matrix_get(a, 1, 0)*matrix_get(b, 0, 1) + matrix_get(a, 1, 1)*matrix_get(b, 1, 1) + matrix_get(a, 1, 2)*matrix_get(b, 2, 1))) status = 1;
    if (matrix_set(axb, 1, 2, matrix_get(a, 1, 0)*matrix_get(b, 0, 2) + matrix_get(a, 1, 1)*matrix_get(b, 1, 2) + matrix_get(a, 1, 2)*matrix_get(b, 2, 2))) status = 1;
    if (matrix_set(axb, 2, 0, matrix_get(a, 2, 0)*matrix_get(b, 0, 0) + matrix_get(a, 2, 1)*matrix_get(b, 1, 0) + matrix_get(a, 2, 2)*matrix_get(b, 2, 0))) status = 1;
    if (matrix_set(axb, 2, 1, matrix_get(a, 2, 0)*matrix_get(b, 0, 1) + matrix_get(a, 2, 1)*matrix_get(b, 1, 1) + matrix_get(a, 2, 2)*matrix_get(b, 2, 1))) status = 1;
    if (matrix_set(axb, 2, 2, matrix_get(a, 2, 0)*matrix_get(b, 0, 2) + matrix_get(a, 2, 1)*matrix_get(b, 1, 2) + matrix_get(a, 2, 2)*matrix_get(b, 2, 2))) status = 1;

    if (status == 1) {
        matrix_free(axb);
        return 1;
    } else {
        return 0;
    }
}

/* ------------------------------------------------------------------------- */

int matrix3_mulmul(matrix_t *axbxc, const matrix_t a, const matrix_t b, const matrix_t c) {
    matrix_t bxc = {0};
    if (matrix3_mul(&bxc, b, c)) return 1;
    if (matrix3_mul(axbxc, a, bxc)) return 1;
    return 0;
}

/* ------------------------------------------------------------------------- */

int matrix2_det(double *det, const matrix_t mat);

/* ------------------------------------------------------------------------- */

int matrix3_det(double *det, const matrix_t mat);

/* ------------------------------------------------------------------------- */

int matrix2_inv(matrix_t *inverse, const matrix_t mat);

/* ------------------------------------------------------------------------- */

int matrix3_inv(matrix_t *inverse, const matrix_t mat);

/* ------------------------------------------------------------------------- */

bool matrix_equal(const matrix_t mat1, const matrix_t mat2, const double tolerance) {
    bool equal = true;
    size_t i = 0;
    size_t j = 0;
    if (mat1.rows != mat2.rows || mat1.cols != mat2.cols) return false;
    if (!matrix_is_allocated(&mat1) || !matrix_is_allocated(&mat2)) return false;

    for (i = 0; i < mat1.rows; i++) {
        for (j = 0; j < mat1.cols; j++) {
            equal = fabs(matrix_get(mat1, i, j) - matrix_get(mat2, i, j)) > tolerance;
            if (!equal) return false;
        }
    }
    return true;
}

/* ------------------------------------------------------------------------- */
