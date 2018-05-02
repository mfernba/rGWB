// Array de array 3D...

#include "csmArrPoint3D.h"

#include "csmarrayc.inl"
#include "csmassert.inl"
#include "csmmem.inl"

struct i_point_t
{
    double x, y, z;
};

// ---------------------------------------------------------------------------

CONSTRUCTOR(static struct i_point_t *, i_new_point, (double x, double y, double z))
{
    struct i_point_t *point;
    
    point = MALLOC(struct i_point_t);
    
    point->x = x;
    point->y = y;
    point->z = z;
    
    return point;
}

// ---------------------------------------------------------------------------

CONSTRUCTOR(static struct i_point_t *, i_copy_point, (const struct i_point_t *point))
{
    assert_no_null(point);
    return i_new_point(point->x, point->y, point->z);
}

// ---------------------------------------------------------------------------

static void i_free_point(struct i_point_t **point)
{
    FREE_PP(point, struct i_point_t);
}

// ---------------------------------------------------------------------------

csmArrPoint3D *csmArrPoint3D_new(unsigned long no_elems)
{
    csmArrayStruct(i_point_t) *array;
    unsigned long i;
    
    array = csmarrayc_new_st_array(no_elems, i_point_t);
    
    if (no_elems > 0)
    {
        for (i = 0; i < no_elems; i++)
            csmarrayc_set_st(array, i, NULL, i_point_t);
    }
    
    return (csmArrPoint3D *)array;
}

// ---------------------------------------------------------------------------

csmArrPoint3D *csmArrPoint3D_copy(const csmArrPoint3D *array)
{
    return (csmArrPoint3D *)csmarrayc_copy_st_array((csmArrayStruct(i_point_t) *)array, i_point_t, i_copy_point);
}

// ---------------------------------------------------------------------------

void csmArrPoint3D_free(csmArrPoint3D **array)
{
    csmarrayc_free_st((csmArrayStruct(i_point_t) **)array, i_point_t, i_free_point);
}

// ---------------------------------------------------------------------------

unsigned long csmArrPoint3D_count(const csmArrPoint3D *array)
{
    return csmarrayc_count_st((csmArrayStruct(i_point_t) *)array, i_point_t);
}

// ---------------------------------------------------------------------------

void csmArrPoint3D_append(csmArrPoint3D *array, double x, double y, double z)
{
    struct i_point_t *point;
    
    point = i_new_point(x, y, z);
    csmarrayc_append_element_st((csmArrayStruct(i_point_t) *)array, point, i_point_t);
}

// ---------------------------------------------------------------------------

void csmArrPoint3D_delete(csmArrPoint3D *array, unsigned long idx)
{
    csmarrayc_delete_element_st((csmArrayStruct(i_point_t) *)array, idx, i_point_t, i_free_point);
}

// ---------------------------------------------------------------------------

void csmArrPoint3D_set(csmArrPoint3D *array, unsigned long idx, double x, double y, double z)
{
    struct i_point_t *point;
    
    point = csmarrayc_get_st((csmArrayStruct(i_point_t) *)array, idx, i_point_t);
    
    if (point == NULL)
    {
        point = i_new_point(x, y, z);
        csmarrayc_set_st((csmArrayStruct(i_point_t) *)array, idx, point, i_point_t);
    }
    else
    {
        point->x = x;
        point->y = y;
        point->z = z;
    }
}

// ---------------------------------------------------------------------------

void csmArrPoint3D_get(const csmArrPoint3D *array, unsigned long idx, double *x, double *y, double *z)
{
    const struct i_point_t *point;

    point = csmarrayc_get_const_st((const csmArrayStruct(i_point_t) *)array, idx, i_point_t);
    assert_no_null(point);
	
	ASSIGN_OPTIONAL_VALUE(x, point->x);
	ASSIGN_OPTIONAL_VALUE(y, point->y);
	ASSIGN_OPTIONAL_VALUE(z, point->z);
}

// ---------------------------------------------------------------------------

void csmArrPoint3D_invert(csmArrPoint3D *array)
{
    csmarrayc_invert((csmArrayStruct(i_point_t) *)array, i_point_t);
}
