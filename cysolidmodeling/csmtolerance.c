// Tolerances...

#include "csmtolerance.inl"

// --------------------------------------------------------------------------------

double csmtolerance_equal_coords(void)
{
    return 1.e-6;
}

// --------------------------------------------------------------------------------

double csmtolerance_angle_rad(void)
{
    return 1.e-6;
}

// --------------------------------------------------------------------------------

double csmtolerance_dot_product_parallel_vectors(void)
{
    return 1.e-6;
}

// --------------------------------------------------------------------------------

double csmtolerance_coplanarity(void)
{
    return 1.e-6;
}

// --------------------------------------------------------------------------------

double csmtolerance_relative_position_over_edge(void)
{
    return 1.e-6;
}


// --------------------------------------------------------------------------------

double csmtolerance_null_vector(void)
{
    return 1.e-10;
}

// --------------------------------------------------------------------------------

double csmtolerance_point_in_loop_boundary(void)
{
    return 1.e-6;
}

// --------------------------------------------------------------------------------

double csmtolerance_bbox_absolute_tolerance(void)
{
    return 0.01;
}
