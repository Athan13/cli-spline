#ifndef SPLINE_CALC_H
# define SPLINE_CALC_H

#include <gsl/gsl_matrix.h>

gsl_matrix* get_path(gsl_vector* pebbles_xy, size_t total_x, size_t total_y);

#endif