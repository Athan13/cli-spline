#include <stdio.h>
#include <math.h>

#include <gsl/gsl_matrix.h>

#include "../src/spline-calc.h"

// Utility function for checking equality of matrices given floating point arithmetic errors
int gsl_matrix_approxeq(const gsl_matrix* a, const gsl_matrix* b, double tol) {
    if (a->size1 != b->size1 || a->size2 != b->size2) {
        return 0;
    }

    for (int i = 0; i < a->size1; i++) {
        for (int j = 0; j < a->size2; j++) {
            if ( fabs(gsl_matrix_get(a, i, j) - gsl_matrix_get(b, i, j)) > tol ) return 0;
        }
    }    
    return 1;
}

void print_matrix(const gsl_matrix* a) {
    for (int i = 0; i < a->size1; i++) {
        for (int j = 0; j < a->size2; j++) {
            printf("%f ", gsl_matrix_get(a, i, j));
        }
        printf("\n");
    }
    printf("\n");
}

void test_generate_A_matrix_pass() {
    double ref_matrix_data[] = {
        1., 0., 0., 0.,
        1., 4., 1., 0.,
        0., 1., 4., 1.,
        0., 0., 0., 1.
    };

    gsl_matrix_view ref_matrix = gsl_matrix_view_array(ref_matrix_data, 4, 4);
    gsl_matrix* gen_matrix = generate_A_matrix(4);

    if (gsl_matrix_equal(gen_matrix, &ref_matrix.matrix)) {
        printf("TEST PASSED: test_generate_A_matrix_pass().\n"); 
    } else {
        printf("TEST FAILED: test_generate_A_matrix_pass().\n Generated matrix was:\n");
        print_matrix(gen_matrix);
        printf("\nbut reference matrix was:\n");
        print_matrix(&ref_matrix.matrix);
    }

    gsl_matrix_free(gen_matrix);
}

void test_generate_A_matrix_fail() {
    gsl_matrix* a_matrix = generate_A_matrix(1);
    if (a_matrix == NULL) {
        printf("TEST PASSED: test_generate_A_matrix_fail().\n");
    } else {
        printf("TEST FAILED: test_generate_A_matrix_fail() - a_matrix should be NULL but instead was:\n");
        print_matrix(a_matrix);
        gsl_matrix_free(a_matrix);
    }
}

void test_get_cubic_coeffs1() {
    double xs[] = {1., 1., 1., 1., 1.};
    gsl_vector_view x_coords = gsl_vector_view_array(xs, 5); 
    gsl_matrix* gen_coeffs_m = gsl_matrix_alloc(5, 4);
    int result = get_cubic_coeffs(gen_coeffs_m, &x_coords.vector);

    double ref_coeffs[] = {
        1., 0., 0., 0.,
        1., 0., 0., 0.,
        1., 0., 0., 0.,
        1., 0., 0., 0.
    };
    gsl_matrix_view ref_coeffs_m = gsl_matrix_view_array(ref_coeffs, 4, 4);

    if ( result == 0 && gsl_matrix_approxeq(gen_coeffs_m, &ref_coeffs_m.matrix, 1e-5) ) {
        printf("TEST PASSED: test_get_cubic_coeffs1()\n");
    } else {
        printf("TEST FAILED: test_get_cubic_coeffs1(). Expected vs actual:\n");
        print_matrix(&ref_coeffs_m.matrix);
        print_matrix(gen_coeffs_m);
    }

    gsl_matrix_free(gen_coeffs_m);
}

void test_get_cubic_coeffs2() {
    double xs[] = {1., 3., 5., 2., 6., 0., 1., 9., 4.};
    gsl_vector_view x_coords = gsl_vector_view_array(xs, 9);
    gsl_matrix* gen_coeffs_m = gsl_matrix_alloc(9, 4);
    int result = get_cubic_coeffs(gen_coeffs_m, &x_coords.vector);

    double ref_coeffs[] = {
        1., 1.448913843888071, 0., 0.5510861561119292,
        3., 3.102172312223859, 1.653258468335788, -2.755430780559646,
        5., -1.85760309278350, -6.613033873343151, 5.470636966126656,
        2., 1.328240058910163,  9.798877025036818, -7.12711708394698,
        6., -0.4553571428571441, -11.58247422680412, 6.037831369661267,
        0., -5.506811487481590, 6.531019882179676, -0.02420839469808496,
        1., 7.482603092783505, 6.458394698085421, -5.940997790868926,
        9., 2.576399116347571, -11.36459867452136, 3.788199558173786
    };
    gsl_matrix_view ref_coeffs_m = gsl_matrix_view_array(ref_coeffs, 8, 4);
    
    if ( result == 0 && gsl_matrix_approxeq(gen_coeffs_m, &ref_coeffs_m.matrix, 1e-5) ) {
        printf("TEST PASSED: test_get_cubic_coeffs2()\n");
    } else {
        printf("TEST FAILED: test_get_cubic_coeffs2(). Expected vs actual:\n");
        print_matrix(&ref_coeffs_m.matrix);
        print_matrix(gen_coeffs_m);
    }

    gsl_matrix_free(gen_coeffs_m);
}

void test_get_cubic_coeffs_fail() {
    double xs[] = {1., 3.};
    gsl_vector_view x_coords = gsl_vector_view_array(xs, 2);
    gsl_matrix* gen_coeffs_m = gsl_matrix_alloc(2, 4);
    int result = get_cubic_coeffs(gen_coeffs_m, &x_coords.vector);

    if (result == 1) {
        printf("TEST PASSED: test_generate_A_matrix_fail().\n");
    } else {
        printf("TEST FAILED: test_generate_A_matrix_fail() - a_matrix should be NULL but instead was:\n");
        print_matrix(gen_coeffs_m);
    }

    gsl_matrix_free(gen_coeffs_m);
}

int main(int argc, char** argv) {
    test_generate_A_matrix_pass();
    test_generate_A_matrix_fail();

    test_get_cubic_coeffs1();
    test_get_cubic_coeffs2();
    test_get_cubic_coeffs_fail();

    return 0;
}
