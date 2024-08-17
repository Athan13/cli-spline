#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

/*
    Generates matrix A (see README/How It Works). Because of how we defined the t's, we set
    all h_0 = h_1 = ... = h_{n - 1} = 1.
*/
gsl_matrix* generate_A_matrix(size_t n) {
    // Need at least three points for cubic spline interpolation
    if (n <= 2) {
        fprintf(stderr, "Number of points must be greater than two (generate_A_matrix)");
        return NULL;
    }

    gsl_matrix* a_matrix = gsl_matrix_calloc(n, n);
    gsl_matrix_set(a_matrix, 1, 1, 1.);
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
    such that x_i(t) = a_i + b_i*t + c_i*t^2 + d_i*t^3
*/
gsl_matrix* get_cubics(const gsl_vector* pebbles_coord, size_t num_pebbles) {
    gsl_matrix* cubics_coeffs = gsl_matrix_alloc(num_pebbles, 4);

    if (cubics_coeffs == NULL)
        return NULL;

    // Find a's (constant of cubic equation) --> copy first n-1 elements of pebbles_coord into first column
    // of coefficients matrix
    gsl_vector_view col0 = gsl_matrix_row(cubics_coeffs, 0);
    gsl_vector_memcpy(&col0.vector, pebbles_coord);

    // Find c's (coefficient of t^2) by solving relevant Ax = b using LU decomposition
    gsl_matrix* a_matrix = generate_A_matrix(num_pebbles);

    gsl_vector* b_vector = gsl_vector_alloc(num_pebbles);
    double prev_a, curr_a, next_a;
    for (size_t i = 1; i < num_pebbles - 1; i++) {
        prev_a = gsl_matrix_get(cubics_coeffs, i - 1, 0);
        curr_a = gsl_matrix_get(cubics_coeffs, i, 0);
        next_a = gsl_matrix_get(cubics_coeffs, i + 1, 0);
        gsl_vector_set(b_vector, i, 3 * next_a - 6 * curr_a - 3 * prev_a);
    }
    gsl_vector_set(b_vector, 0, 0.);
    gsl_vector_set(b_vector, num_pebbles - 1, 0.);

    gsl_vector_view col3 = gsl_matrix_row(cubics_coeffs, 2);

    gsl_permutation* p = gsl_permutation_alloc(num_pebbles); int s; // Needed for LU decomposition
    gsl_linalg_LU_decomp(a_matrix, p, &s);
    gsl_linalg_LU_solve(a_matrix, p, b_vector, &col3.vector);

    gsl_matrix_free(a_matrix);
    gsl_vector_free(b_vector);

    return cubics_coeffs;
}

/*
    Takes in a series of pebbles and constructs a path for ants to follow.
    Pebbles are represented as xy-coords in pebbles_xy, total_x and total_y are the
    total width and height of the game window respectively.
    The resulting path is returned in the form of a gsl_vector.
*/
gsl_vector* get_path(const gsl_vector* pebbles_xy, size_t total_x, size_t total_y) {
    size_t num_pebbles = pebbles_xy->size / 2;

    gsl_vector_const_view pebbles_x_view = gsl_vector_const_subvector_with_stride(pebbles_xy, 0, 2, num_pebbles);
    gsl_vector_const_view pebbles_y_view = gsl_vector_const_subvector_with_stride(pebbles_xy, 1, 2, num_pebbles);

}
