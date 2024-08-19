#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

/*
    Generates matrix A (see README/How It Works). Because of how we defined the t's, we set
    all h_0 = h_1 = ... = h_{n - 1} = 1.
*/
gsl_matrix* generate_A_matrix(size_t n) {
    // Need at least three points for cubic spline interpolation
    if (n <= 2) {
        fprintf(stderr, "Number of points must be greater than two (generate_A_matrix)\n");
        return NULL;
    }

    gsl_matrix* a_matrix = gsl_matrix_calloc(n, n);
    gsl_matrix_set(a_matrix, 0, 0, 1.);
    gsl_matrix_set(a_matrix, n - 1, n - 1, 1.);

    for (size_t i = 1; i < n - 1; i++) {
        gsl_matrix_set(a_matrix, i, i - 1, 1.);
        gsl_matrix_set(a_matrix, i, i, 4.);
        gsl_matrix_set(a_matrix, i, i + 1, 1.);
    }

    return a_matrix;
}

/*
    Returns matrix of cubic coefficients, where the i'th row is of the form [a_i, b_i, c_i, d_i]
    such that x_i(t) = a_i + b_i*t + c_i*t^2 + d_i*t^3.

    `cubics_coeffs` must have dimensions size1 = pebbles_coord->size, size2 = 4.
*/
int get_cubic_coeffs(gsl_matrix* cubics_coeffs, const gsl_vector* pebbles_coord) {
    size_t num_pebbles = pebbles_coord->size;

    // Find a's (constant of cubic equation) --> copy pebbles_coord into first column
    // of coefficients matrix
    gsl_vector_view col0 = gsl_matrix_column(cubics_coeffs, 0);
    gsl_vector_add(&col0.vector, pebbles_coord);

    // Find c's (coefficient of t^2) by solving relevant Ax = b using LU decomposition
    gsl_matrix* a_matrix = generate_A_matrix(num_pebbles);
    if (a_matrix == NULL) return 1;

    gsl_vector* b_vector = gsl_vector_alloc(num_pebbles);
    double prev_a, curr_a, next_a;
    for (size_t i = 1; i < num_pebbles - 1; i++) {
        prev_a = gsl_matrix_get(cubics_coeffs, i - 1, 0);
        curr_a = gsl_matrix_get(cubics_coeffs, i, 0);
        next_a = gsl_matrix_get(cubics_coeffs, i + 1, 0);
        gsl_vector_set(b_vector, i, 3 * next_a - 6 * curr_a + 3 * prev_a);
    }
    gsl_vector_set(b_vector, 0, 0.);
    gsl_vector_set(b_vector, num_pebbles - 1, 0.);

    gsl_vector_view col2 = gsl_matrix_column(cubics_coeffs, 2);
    gsl_permutation* p = gsl_permutation_alloc(num_pebbles); int s; // needed for LU decomp
    gsl_linalg_LU_decomp(a_matrix, p, &s);
    gsl_linalg_LU_solve(a_matrix, p, b_vector, &col2.vector);

    gsl_matrix_free(a_matrix);
    gsl_vector_free(b_vector);
    gsl_permutation_free(p);

    // Find b's and d's (coefficient of t and t^3)
    double curr_c, next_c, curr_b, curr_d;
    for (size_t i = 0; i < num_pebbles - 1; i++) {
        curr_a = gsl_matrix_get(cubics_coeffs, i, 0);
        next_a = gsl_matrix_get(cubics_coeffs, i + 1, 0);

        curr_c = gsl_matrix_get(cubics_coeffs, i, 2);
        next_c = gsl_matrix_get(cubics_coeffs, i + 1, 2);

        curr_b = next_a - curr_a - (2*curr_c + next_c) / 3;
        curr_d = (next_c - curr_c) / 3;

        gsl_matrix_set(cubics_coeffs, i, 1, curr_b);
        gsl_matrix_set(cubics_coeffs, i, 3, curr_d);
    }

    cubics_coeffs->size1 -= 1;

    return 0;
}

/*
    Evaluates a cubic spline at `t_sample_resolution` places. Result is a matrix that describes
    the path of the spline in row major order.
*/
int spline_eval(const gsl_matrix* cubics_coeffs, size_t t_sample_resolution, gsl_matrix* eval_matrix) {
    gsl_matrix* t_matrix = gsl_matrix_alloc(4, t_sample_resolution);
    gsl_vector_view row0 = gsl_matrix_row(t_matrix, 0);
    gsl_vector_set_all(&row0.vector, 1);

    for (size_t j = 0; j < t_sample_resolution; j++) {
        double t = j / t_sample_resolution;
        gsl_matrix_set(t_matrix, 1, j, t);
        gsl_matrix_set(t_matrix, 2, j, t * t);
        gsl_matrix_set(t_matrix, 3, j, t * t * t);
    }

    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1., cubics_coeffs, t_matrix, 0., eval_matrix);

    gsl_matrix_free(t_matrix);
    return 0;
}

/*
    Takes in a series of pebbles and constructs a path for ants to follow.

    Pebbles are represented as xy-coords in pebbles_xy, `t_sample_resolution` is the
    resolution at which the splines are evaluated (number of samples between one pebble and the next).

    Result is stored in `path_matrix`, which must be of dimensions size1 = pebbles_xy->size - 2 and
    size2 = t_sample_resolution.
*/
int create_path(gsl_matrix* path_matrix, const gsl_vector* pebbles_xy, size_t t_sample_resolution) {
    size_t num_pebbles = pebbles_xy->size / 2;
    gsl_vector_const_view pebbles_x_view = gsl_vector_const_subvector_with_stride(pebbles_xy, 0, 2, num_pebbles);
    gsl_vector_const_view pebbles_y_view = gsl_vector_const_subvector_with_stride(pebbles_xy, 1, 2, num_pebbles);

    gsl_matrix* x_coeffs = gsl_matrix_alloc(num_pebbles, 4);
    gsl_matrix* y_coeffs = gsl_matrix_alloc(num_pebbles, 4);

    get_cubic_coeffs(x_coeffs, &pebbles_x_view.vector);
    get_cubic_coeffs(y_coeffs, &pebbles_y_view.vector);

    // The top half of the matrix spline_eval_m becomes the x coords of the path, while
    // the bottom half becomes the y coords (both in row major order)
    gsl_matrix_view x_path = gsl_matrix_submatrix(path_matrix, 0, 0, x_coeffs->size1, t_sample_resolution);
    gsl_matrix_view y_path = gsl_matrix_submatrix(path_matrix, x_coeffs->size1, 0, x_coeffs->size1, t_sample_resolution);

    spline_eval(x_coeffs, t_sample_resolution, &x_path.matrix);
    spline_eval(y_coeffs, t_sample_resolution, &y_path.matrix);

    gsl_matrix_free(x_coeffs);
    gsl_matrix_free(y_coeffs);

    return 0;
}
