#ifndef SPLINE_CALC_H
# define SPLINE_CALC_H

#include <gsl/gsl_matrix.h>

gsl_matrix* generate_A_matrix(size_t n); 

int get_cubic_coeffs(gsl_matrix* cubics_coeffs, const gsl_vector* pebbles_coord);

int create_path(gsl_matrix* path_matrix, const gsl_vector* pebbles_xy, size_t t_sample_resolution);

#endif