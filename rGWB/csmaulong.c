// Array de array 3D...

#include "csmaulong.h"

#include "csmarrayc.h"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

struct i_ulong_t
{
    unsigned long value;
};

// ---------------------------------------------------------------------------

CONSTRUCTOR(static struct i_ulong_t *, i_new_ulong_value, (unsigned long value))
{
    struct i_ulong_t *ulong_value;
    
    ulong_value = MALLOC(struct i_ulong_t);
    
    ulong_value->value = value;
    
    return ulong_value;
}

// ---------------------------------------------------------------------------

CONSTRUCTOR(static struct i_ulong_t *, i_copy_ulong_value, (const struct i_ulong_t *ulong_value))
{
    assert_no_null(ulong_value);
    return i_new_ulong_value(ulong_value->value);
}

// ---------------------------------------------------------------------------

static void i_free_ulong_value(struct i_ulong_t **ulong_value)
{
    FREE_PP(ulong_value, struct i_ulong_t);
}

// ---------------------------------------------------------------------------

csmArrULong *csmArrULong_new(unsigned long no_elems)
{
    csmArrayStruct(i_ulong_t) *array;
    unsigned long i;
    
    array = csmarrayc_new_st_array(no_elems, i_ulong_t);
    
    if (no_elems > 0)
    {
        for (i = 0; i < no_elems; i++)
            csmarrayc_set_st(array, i, NULL, i_ulong_t);
    }
    
    return (csmArrULong *)array;
}

// ---------------------------------------------------------------------------

csmArrULong *csmArrULong_copy(const csmArrULong *array)
{
    return (csmArrULong *)csmarrayc_copy_st_array((csmArrayStruct(i_ulong_t) *)array, i_ulong_t, i_copy_ulong_value);
}

// ---------------------------------------------------------------------------

void csmArrULong_free(csmArrULong **array)
{
    csmarrayc_free_st((csmArrayStruct(i_ulong_t) **)array, i_ulong_t, i_free_ulong_value);
}

// ---------------------------------------------------------------------------

unsigned long csmArrULong_count(const csmArrULong *array)
{
    return csmarrayc_count_st((csmArrayStruct(i_ulong_t) *)array, i_ulong_t);
}

// ---------------------------------------------------------------------------

void csmArrULong_append(csmArrULong *array, unsigned long value)
{
    struct i_ulong_t *ulong_value;
    
    ulong_value = i_new_ulong_value(value);
    csmarrayc_append_element_st((csmArrayStruct(i_ulong_t) *)array, ulong_value, i_ulong_t);
}

// ---------------------------------------------------------------------------

void csmArrULong_delete(csmArrULong *array, unsigned long idx)
{
    csmarrayc_delete_element_st((csmArrayStruct(i_ulong_t) *)array, idx, i_ulong_t, i_free_ulong_value);
}

// ---------------------------------------------------------------------------

void csmArrULong_set(csmArrULong *array, unsigned long idx, unsigned long value)
{
    struct i_ulong_t *ulong_value;
    
    ulong_value = csmarrayc_get_st((csmArrayStruct(i_ulong_t) *)array, idx, i_ulong_t);
    
    if (ulong_value == NULL)
    {
        ulong_value = i_new_ulong_value(value);
        csmarrayc_set_st((csmArrayStruct(i_ulong_t) *)array, idx, ulong_value, i_ulong_t);
    }
    else
    {
        ulong_value->value = value;
    }
}

// ---------------------------------------------------------------------------

unsigned long csmArrULong_get(const csmArrULong *array, unsigned long idx)
{
    const struct i_ulong_t *ulong_value;

    ulong_value = csmarrayc_get_const_st((const csmArrayStruct(i_ulong_t) *)array, idx, i_ulong_t);
    assert_no_null(ulong_value);
    
    return ulong_value->value;
}
