// Tolerances...

#include "csmtolerance.inl"

#include "csmassert.inl"
#include "csmmem.inl"

struct csmtolerance_t
{
    double tolerance_equal_coords;
    double tolerance_equal_angle_rad;
    double tolerance_dot_product_parallel_vectors;
    double tolerance_coplanarity;
    double tolerance_null_vector;
    double tolerance_bbox_absolute_tolerance;
};

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmtolerance_t *, i_new, (
                        double tolerance_equal_coords,
                        double tolerance_equal_angle_rad,
                        double tolerance_dot_product_parallel_vectors,
                        double tolerance_coplanarity,
                        double tolerance_null_vector,
                        double tolerance_bbox_absolute_tolerance))
{
    struct csmtolerance_t *tolerance;
    
    tolerance = MALLOC(struct csmtolerance_t);
    
    tolerance->tolerance_equal_coords = tolerance_equal_coords;
    tolerance->tolerance_equal_angle_rad = tolerance_equal_angle_rad;
    tolerance->tolerance_dot_product_parallel_vectors = tolerance_dot_product_parallel_vectors;
    tolerance->tolerance_coplanarity = tolerance_coplanarity;
    tolerance->tolerance_null_vector = tolerance_null_vector;
    tolerance->tolerance_bbox_absolute_tolerance = tolerance_bbox_absolute_tolerance;
    
    return tolerance;
}

// --------------------------------------------------------------------------------

struct csmtolerance_t *csmtolerance_new(void)
{
    double tolerance_equal_coords;
    double tolerance_equal_angle_rad;
    double tolerance_dot_product_parallel_vectors;
    double tolerance_coplanarity;
    double tolerance_null_vector;
    double tolerance_bbox_absolute_tolerance;

/*
    tolerance_equal_coords = 1.e-6;
    tolerance_equal_angle_rad = 1.e-6;
    tolerance_dot_product_parallel_vectors = 1.e-6;
    tolerance_coplanarity = csmtolerance_default_coplanarity();
    tolerance_relative_position_over_edge = 1.e-6;
    tolerance_null_vector = csmtolerance_default_null_vector();
    tolerance_point_in_loop_boundary = 1.e-6;
    tolerance_bbox_absolute_tolerance = 0.01;
 */
    tolerance_equal_coords = 1.e-6;
    tolerance_equal_angle_rad = 1.e-6;
    tolerance_dot_product_parallel_vectors = 1.e-6;
    tolerance_coplanarity = csmtolerance_default_coplanarity();
    tolerance_null_vector = csmtolerance_default_null_vector();
    tolerance_bbox_absolute_tolerance = 0.01;
    
    return i_new(
                tolerance_equal_coords,
                tolerance_equal_angle_rad,
                tolerance_dot_product_parallel_vectors,
                tolerance_coplanarity,
                tolerance_null_vector,
                tolerance_bbox_absolute_tolerance);
}

// --------------------------------------------------------------------------------

void csmtolerance_free(struct csmtolerance_t **tolerance)
{
    assert_no_null(tolerance);
    assert_no_null(*tolerance);
    
    FREE_PP(tolerance, struct csmtolerance_t);
}

// --------------------------------------------------------------------------------

double csmtolerance_equal_coords(const struct csmtolerance_t *tolerance)
{
    assert_no_null(tolerance);
    return tolerance->tolerance_equal_coords;
}

// --------------------------------------------------------------------------------

double csmtolerance_angle_rad(const struct csmtolerance_t *tolerance)
{
    assert_no_null(tolerance);
    return tolerance->tolerance_equal_angle_rad;
}

// --------------------------------------------------------------------------------

double csmtolerance_dot_product_parallel_vectors(const struct csmtolerance_t *tolerance)
{
    assert_no_null(tolerance);
    return tolerance->tolerance_dot_product_parallel_vectors;
}

// --------------------------------------------------------------------------------

double csmtolerance_coplanarity(const struct csmtolerance_t *tolerance)
{
    assert_no_null(tolerance);
    return tolerance->tolerance_coplanarity;
}

// --------------------------------------------------------------------------------

double csmtolerance_null_vector(const struct csmtolerance_t *tolerance)
{
    assert_no_null(tolerance);
    return tolerance->tolerance_null_vector;
}

// --------------------------------------------------------------------------------

double csmtolerance_bbox_absolute_tolerance(const struct csmtolerance_t *tolerance)
{
    assert_no_null(tolerance);
    return tolerance->tolerance_bbox_absolute_tolerance;
}

// --------------------------------------------------------------------------------

double csmtolerance_default_null_vector(void)
{
    return 1.e-10;
}

// --------------------------------------------------------------------------------

double csmtolerance_default_coplanarity(void)
{
    return 1.e-6;
}
