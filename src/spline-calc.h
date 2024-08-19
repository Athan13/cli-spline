#ifndef SPLINE_CALC_H
# define SPLINE_CALC_H

#include <gsl/gsl_matrix.h>

gsl_matrix* generate_A_matrix(size_t n); 

gsl_matrix* get_cubic_coeffs(const gsl_vector* pebbles_coord);

gsl_matrix* get_path(gsl_vector* pebbles_xy, size_t total_x, size_t total_y);

#endif