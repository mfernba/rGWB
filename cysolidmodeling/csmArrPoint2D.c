// Array de array 2D...

#include "csmArrPoint2D.h"

#include "csmarrayc.inl"
#include "csmassert.inl"
#include "csmmem.inl"
#include "csmmath.inl"
#include "csmmath.tli"

#include <float.h>

struct i_point_t
{
    double x, y;
};

struct i_searched_point_t
{
    double x, y;
    double tolerance;
    csmArrPoint2D_FPtr_equal_points func_equal_points;
};

// ---------------------------------------------------------------------------

CONSTRUCTOR(static struct i_point_t *, i_new_point, (double x, double y))
{
    struct i_point_t *point;
    
    point = MALLOC(struct i_point_t);
    
    point->x = x;
    point->y = y;
    
    return point;
}

// ---------------------------------------------------------------------------

CONSTRUCTOR(static struct i_point_t *, i_copy_point, (const struct i_point_t *point))
{
    assert_no_null(point);
    return i_new_point(point->x, point->y);
}

// ---------------------------------------------------------------------------

static void i_free_point(struct i_point_t **point)
{
    FREE_PP(point, struct i_point_t);
}

// ---------------------------------------------------------------------------

csmArrPoint2D *csmArrPoint2D_new(unsigned long no_elems)
{
    csmArrayStruct(i_point_t) *array;
    unsigned long i;
    
    array = csmarrayc_new_st_array(no_elems, i_point_t);
    
    if (no_elems > 0)
    {
        for (i = 0; i < no_elems; i++)
            csmarrayc_set_st(array, i, NULL, i_point_t);
    }
    
    return (csmArrPoint2D *)array;
}

// ---------------------------------------------------------------------------

csmArrPoint2D *csmArrPoint2D_copy(const csmArrPoint2D *array)
{
    return (csmArrPoint2D *)csmarrayc_copy_st_array((csmArrayStruct(i_point_t) *)array, i_point_t, i_copy_point);
}

// ---------------------------------------------------------------------------

void csmArrPoint2D_free(csmArrPoint2D **array)
{
    csmarrayc_free_st((csmArrayStruct(i_point_t) **)array, i_point_t, i_free_point);
}

// ---------------------------------------------------------------------------

unsigned long csmArrPoint2D_count(const csmArrPoint2D *array)
{
    return csmarrayc_count_st((csmArrayStruct(i_point_t) *)array, i_point_t);
}

// ---------------------------------------------------------------------------

void csmArrPoint2D_append(csmArrPoint2D *array, double x, double y)
{
    struct i_point_t *point;
    
    point = i_new_point(x, y);
    csmarrayc_append_element_st((csmArrayStruct(i_point_t) *)array, point, i_point_t);
}

// ---------------------------------------------------------------------------

void csmArrPoint2D_delete(csmArrPoint2D *array, unsigned long idx)
{
    csmarrayc_delete_element_st((csmArrayStruct(i_point_t) *)array, idx, i_point_t, i_free_point);
}

// ---------------------------------------------------------------------------

void csmArrPoint2D_set(csmArrPoint2D *array, unsigned long idx, double x, double y)
{
    struct i_point_t *point;
    
    point = csmarrayc_get_st((csmArrayStruct(i_point_t) *)array, idx, i_point_t);
    
    if (point == NULL)
    {
        point = i_new_point(x, y);
        csmarrayc_set_st((csmArrayStruct(i_point_t) *)array, idx, point, i_point_t);
    }
    else
    {
        point->x = x;
        point->y = y;
    }
}

// ---------------------------------------------------------------------------

void csmArrPoint2D_get(const csmArrPoint2D *array, unsigned long idx, double *x, double *y)
{
    const struct i_point_t *point;

    point = csmarrayc_get_const_st((const csmArrayStruct(i_point_t) *)array, idx, i_point_t);
    assert_no_null(point);
	
	ASSIGN_OPTIONAL_VALUE(x, point->x);
	ASSIGN_OPTIONAL_VALUE(y, point->y);
}

// ---------------------------------------------------------------------------

void csmArrPoint2D_invert(csmArrPoint2D *array)
{
    csmarrayc_invert((csmArrayStruct(i_point_t) *)array, i_point_t);
}

// ---------------------------------------------------------------------------

double csmArrPoint2D_area(const csmArrPoint2D *array)
{
	double area;
	unsigned long i, num_array;

	num_array = csmArrPoint2D_count(array);
	assert(num_array >= 3);

	area = 0.;

	for (i = 0; i < num_array; i++)
	{
		double x_i, y_i, x_i_1, y_i_1;
		double area_i;

		csmArrPoint2D_get(array, i, &x_i, &y_i);
        
        if (i == num_array - 1)
            csmArrPoint2D_get(array, 0, &x_i_1, &y_i_1);
        else
            csmArrPoint2D_get(array, i + 1, &x_i_1, &y_i_1);

		area_i = x_i * y_i_1 - y_i * x_i_1;
		area += area_i;
	}

	return .5 * area;
}

// ---------------------------------------------------------------------------

void csmArrPoint2D_append_circle_points(
							csmArrPoint2D *array,
							double x, double y, double radius,
							unsigned long no_points_circle,
							CSMBOOL clockwise_orientation)
{
	double increment;
	unsigned long i;	
	
	assert(no_points_circle >= 4);
	
	increment = 2. * CSMMATH_PI / no_points_circle;
	
	if (clockwise_orientation == CSMTRUE)
		increment = -increment;
	
	for (i = 0; i < no_points_circle; i++)
	{
		double angle_i;
		double x_i, y_i;	
		
		angle_i = increment * i;
		
		x_i = x + radius * csmmath_cos(angle_i);
		y_i = y + radius * csmmath_sin(angle_i);
		
		csmArrPoint2D_append(array, x_i, y_i);
	}
}

// ---------------------------------------------------------------------------

void csmArrPoint2D_append_elipse_points(
                            csmArrPoint2D *array,
                            double x, double y, double radius_x, double radius_y,
                            unsigned long no_points_circle,
                            CSMBOOL clockwise_orientation)
{
	double increment;
	unsigned long i;
	
	assert(no_points_circle >= 4);
	
	increment = 2. * CSMMATH_PI / no_points_circle;
	
	if (clockwise_orientation == CSMTRUE)
		increment = -increment;
	
	for (i = 0; i < no_points_circle; i++)
	{
		double angle_i;
		double x_i, y_i;
		
		angle_i = increment * i;
		
		x_i = x + radius_x * csmmath_cos(angle_i);
		y_i = y + radius_y * csmmath_sin(angle_i);
		
		csmArrPoint2D_append(array, x_i, y_i);
	}
}

// ---------------------------------------------------------------------------

static CSMBOOL i_equal_points(const struct i_point_t *point, const struct i_searched_point_t *searched_point)
{
    assert_no_null(point);
    assert_no_null(searched_point);
    
    return searched_point->func_equal_points(point->x, point->y, searched_point->x, searched_point->y);
}
                              
// ---------------------------------------------------------------------------

CSMBOOL csmArrPoint2D_exists_point(
                            const csmArrPoint2D *array,
                            double x, double y,
                            csmArrPoint2D_FPtr_equal_points func_equal_points,
                            unsigned long *idx_opt)
{
    struct i_searched_point_t searched_point;
    
    assert_no_null(func_equal_points);
    
    searched_point.x = x;
    searched_point.y = y;
    searched_point.func_equal_points = func_equal_points;
    
    return csmarrayc_contains_element_st(
                            (csmArrayStruct(i_point_t) *)array, i_point_t,
                            &searched_point, struct i_searched_point_t,
                            i_equal_points,
                            idx_opt);
}

// ---------------------------------------------------------------------------

static CSMBOOL i_equal_points_with_tolerance(const struct i_point_t *point, const struct i_searched_point_t *searched_point)
{
    assert_no_null(point);
    assert_no_null(searched_point);
    
    return csmmath_equal_coords(point->x, point->y, 0., searched_point->x, searched_point->y, 0., searched_point->tolerance);
}

// ---------------------------------------------------------------------------

CSMBOOL csmArrPoint2D_exists_point_with_tolerance(
                            const csmArrPoint2D *array,
                            double x, double y,
                            double tolerance,
                            unsigned long *idx_opt)
{
    struct i_searched_point_t searched_point;
    
    searched_point.x = x;
    searched_point.y = y;
    searched_point.tolerance = tolerance;
    
    return csmarrayc_contains_element_st(
                            (csmArrayStruct(i_point_t) *)array, i_point_t,
                            &searched_point, struct i_searched_point_t,
                            i_equal_points_with_tolerance,
                            idx_opt);
}

// ---------------------------------------------------------------------------

void csmArrPoint2D_bounding_box(const csmArrPoint2D *array, double *x_min, double *y_min, double *x_max, double *y_max)
{
    double x_min_loc, y_min_loc, x_max_loc, y_max_loc;
    unsigned long i, no_points;
    
    no_points = csmArrPoint2D_count(array);
    assert(no_points > 0);
    
    x_min_loc = DBL_MAX;
    y_min_loc = DBL_MAX;
    
    x_max_loc = -DBL_MAX;
    y_max_loc = -DBL_MAX;
    
    for (i = 0; i < no_points; i++)
    {
        double x, y;
        
        csmArrPoint2D_get(array, i, &x, &y);
        
        x_min_loc = CSMMATH_MIN(x_min_loc, x);
        y_min_loc = CSMMATH_MIN(y_min_loc, y);
        
        x_max_loc = CSMMATH_MAX(x_max_loc, x);
        y_max_loc = CSMMATH_MAX(y_max_loc, y);
    }
    
    ASSIGN_OPTIONAL_VALUE(x_min, x_min_loc);
    ASSIGN_OPTIONAL_VALUE(y_min, y_min_loc);
    ASSIGN_OPTIONAL_VALUE(x_max, x_max_loc);
    ASSIGN_OPTIONAL_VALUE(y_max, y_max_loc);
}







