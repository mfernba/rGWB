//
//  csmsave.c
//  rGWB
//
//  Created by Manuel Fernández on 18/6/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmsave.h"
#include "csmsave.inl"

#include "csmarrayc.h"
#include "csmaulong.h"
#include "csmstring.inl"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

struct csmsave_t
{
    struct csmsave_item_t *csmsave_item;
    csmsave_FPtr_free_item func_free_item;
    
    csmsave_FPtr_write_bool func_write_bool;
    csmsave_FPtr_write_uchar func_write_uchar;
    csmsave_FPtr_write_ushort func_write_ushort;
    csmsave_FPtr_write_ulong func_write_ulong;
    csmsave_FPtr_write_double func_write_double;
    csmsave_FPtr_write_float func_write_float;
    csmsave_FPtr_write_string func_write_string;
    
    csmsave_FPtr_read_bool func_read_bool;
    csmsave_FPtr_read_uchar func_read_uchar;
    csmsave_FPtr_read_ushort func_read_ushort;
    csmsave_FPtr_read_ulong func_read_ulong;
    csmsave_FPtr_read_double func_read_double;
    csmsave_FPtr_read_float func_read_float;
    csmsave_FPtr_read_string func_read_string;
    
    csmsave_FPtr_write_string func_write_st_mark;
    csmsave_FPtr_read_string func_read_st_mark;
};

// ----------------------------------------------------------------------------------------------------

struct csmsave_t *csmsave_dontuse_new(
                        struct csmsave_item_t **csmsave_item,
                        csmsave_FPtr_free_item func_free_item,
                        csmsave_FPtr_write_bool func_write_bool,
                        csmsave_FPtr_write_uchar func_write_uchar,
                        csmsave_FPtr_write_ushort func_write_ushort,
                        csmsave_FPtr_write_ulong func_write_ulong,
                        csmsave_FPtr_write_double func_write_double,
                        csmsave_FPtr_write_float func_write_float,
                        csmsave_FPtr_write_string func_write_string,
                        csmsave_FPtr_read_bool func_read_bool,
                        csmsave_FPtr_read_uchar func_read_uchar,
                        csmsave_FPtr_read_ushort func_read_ushort,
                        csmsave_FPtr_read_ulong func_read_ulong,
                        csmsave_FPtr_read_double func_read_double,
                        csmsave_FPtr_read_float func_read_float,
                        csmsave_FPtr_read_string func_read_string,
                        csmsave_FPtr_write_string func_write_st_mark,
                        csmsave_FPtr_read_string func_read_st_mark)
{
    struct csmsave_t *csmsave;
    
    csmsave = MALLOC(struct csmsave_t);
    
    csmsave->csmsave_item = ASSIGN_POINTER_PP_NOT_NULL(csmsave_item, struct csmsave_item_t);
    csmsave->func_free_item = func_free_item;
    
    csmsave->func_write_bool = func_write_bool;
    csmsave->func_write_uchar = func_write_uchar;
    csmsave->func_write_ushort = func_write_ushort;
    csmsave->func_write_ulong = func_write_ulong;
    csmsave->func_write_double = func_write_double;
    csmsave->func_write_float = func_write_float;
    csmsave->func_write_string = func_write_string;
    
    csmsave->func_read_bool = func_read_bool;
    csmsave->func_read_uchar = func_read_uchar;
    csmsave->func_read_ushort = func_read_ushort;
    csmsave->func_read_ulong = func_read_ulong;
    csmsave->func_read_double = func_read_double;
    csmsave->func_read_float = func_read_float;
    csmsave->func_read_string = func_read_string;
    
    csmsave->func_write_st_mark = func_write_st_mark;
    csmsave->func_read_st_mark = func_read_st_mark;
    
    return csmsave;
}

// ----------------------------------------------------------------------------------------------------

void csmsave_free(struct csmsave_t **csmsave)
{
    assert_no_null(csmsave);
    assert_no_null(*csmsave);
    assert_no_null((*csmsave)->func_free_item);
    
    (*csmsave)->func_free_item(&(*csmsave)->csmsave_item);
    
    FREE_PP(csmsave, struct csmsave_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_bool(struct csmsave_t *csmsave, CSMBOOL value)
{
    assert_no_null(csmsave);
    assert(csmsave->func_write_bool);
    
    csmsave->func_write_bool(csmsave->csmsave_item, value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_ushort(struct csmsave_t *csmsave, unsigned short value)
{
    assert_no_null(csmsave);
    assert(csmsave->func_write_ushort);
    
    csmsave->func_write_ushort(csmsave->csmsave_item, value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_uchar(struct csmsave_t *csmsave, unsigned char value)
{
    assert_no_null(csmsave);
    assert(csmsave->func_write_uchar);
    
    csmsave->func_write_uchar(csmsave->csmsave_item, value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_ulong(struct csmsave_t *csmsave, unsigned long value)
{
    assert_no_null(csmsave);
    assert(csmsave->func_write_ulong);
    
    csmsave->func_write_ulong(csmsave->csmsave_item, value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_double(struct csmsave_t *csmsave, double value)
{
    assert_no_null(csmsave);
    assert(csmsave->func_write_double);
    
    csmsave->func_write_double(csmsave->csmsave_item, value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_float(struct csmsave_t *csmsave, float value)
{
    assert_no_null(csmsave);
    assert(csmsave->func_write_float);
    
    csmsave->func_write_float(csmsave->csmsave_item, value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_string(struct csmsave_t *csmsave, const char *value)
{
    assert_no_null(csmsave);
    assert(csmsave->func_write_string);
    
    csmsave->func_write_string(csmsave->csmsave_item, value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_string_optional(struct csmsave_t *csmsave, const char *value)
{
    if (value == NULL)
    {
        csmsave_write_bool(csmsave, CSMFALSE);
    }
    else
    {
        csmsave_write_bool(csmsave, CSMTRUE);
        csmsave_write_string(csmsave, value);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_arr_ulong(struct csmsave_t *csmsave, const csmArrULong *array)
{
    unsigned long i, count;
    
    assert_no_null(csmsave);

    count = csmArrULong_count(array);
    csmsave_write_ulong(csmsave, count);
    
    for (i = 0; i < count; i++)
    {
        unsigned long element;
        
        element = csmArrULong_get(array, i);
        csmsave_write_ulong(csmsave, element);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmsave_dontuse_write_arr_st(
                        struct csmsave_t *csmsave,
                        const csmArrayStruct(csmsave_item_t) *array, const char *type_name,
                        csmsave_FPtr_write_struct func_write_struct)
{
    unsigned long i, count;
    
    assert_no_null(csmsave);
    assert_no_null(func_write_struct);
    
    count = csmarrayc_count_st(array, csmsave_item_t);
    csmsave_write_ulong(csmsave, count);
    
    for (i = 0; i < count; i++)
    {
        const struct csmsave_item_t *element;
        
        element = csmarrayc_get_const_st(array, i, csmsave_item_t);
        csmsave_dontuse_write_st(csmsave, element, type_name, func_write_struct);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmsave_dontuse_write_st(
                        struct csmsave_t *csmsave,
                        const struct csmsave_item_t *item, const char *type_name,
                        csmsave_FPtr_write_struct func_write_struct)
{
    assert_no_null(csmsave);
    assert_no_null(func_write_struct);
    
    if (csmsave->func_write_st_mark != NULL)
        csmsave->func_write_st_mark(csmsave->csmsave_item, type_name);
    
    func_write_struct(item, csmsave);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_dontuse_write_optional_st(
                        struct csmsave_t *csmsave,
                        const struct csmsave_item_t *item, const char *type_name,
                        csmsave_FPtr_write_struct func_write_struct)
{
    assert_no_null(csmsave);
    assert_no_null(func_write_struct);
    
    if (item == NULL)
    {
        csmsave_write_bool(csmsave, CSMFALSE);
    }
    else
    {
        csmsave_write_bool(csmsave, CSMTRUE);
        csmsave_dontuse_write_st(csmsave, item, type_name, func_write_struct);
    }
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmsave_read_bool(struct csmsave_t *csmsave)
{
    assert_no_null(csmsave);
    assert_no_null(csmsave->func_read_bool);

    return csmsave->func_read_bool(csmsave->csmsave_item);
}

// ----------------------------------------------------------------------------------------------------

unsigned char csmsave_read_uchar(struct csmsave_t *csmsave)
{
    assert_no_null(csmsave);
    assert_no_null(csmsave->func_read_uchar);

    return csmsave->func_read_uchar(csmsave->csmsave_item);
}

// ----------------------------------------------------------------------------------------------------

unsigned short csmsave_read_ushort(struct csmsave_t *csmsave)
{
    assert_no_null(csmsave);
    assert_no_null(csmsave->func_read_ushort);

    return csmsave->func_read_ushort(csmsave->csmsave_item);
}

// ----------------------------------------------------------------------------------------------------

unsigned long csmsave_read_ulong(struct csmsave_t *csmsave)
{
    assert_no_null(csmsave);
    assert_no_null(csmsave->func_read_ulong);

    return csmsave->func_read_ulong(csmsave->csmsave_item);
}

// ----------------------------------------------------------------------------------------------------

double csmsave_read_double(struct csmsave_t *csmsave)
{
    assert_no_null(csmsave);
    assert_no_null(csmsave->func_read_double);

    return csmsave->func_read_double(csmsave->csmsave_item);
}

// ----------------------------------------------------------------------------------------------------

float csmsave_read_float(struct csmsave_t *csmsave)
{
    assert_no_null(csmsave);
    assert_no_null(csmsave->func_read_float);

    return csmsave->func_read_float(csmsave->csmsave_item);
}

// ----------------------------------------------------------------------------------------------------

char *csmsave_read_string(struct csmsave_t *csmsave)
{
    assert_no_null(csmsave);
    assert_no_null(csmsave->func_read_string);

    return csmsave->func_read_string(csmsave->csmsave_item);
}

// ----------------------------------------------------------------------------------------------------

char *csmsave_read_string_optional(struct csmsave_t *csmsave)
{
    if (csmsave_read_bool(csmsave) == CSMTRUE)
        return csmsave_read_string(csmsave);
    else
        return NULL;
}

// ----------------------------------------------------------------------------------------------------

csmArrULong *csmsave_read_arr_ulong(struct csmsave_t *csmsave)
{
    csmArrULong *array;
    unsigned long i, count;
    
    assert_no_null(csmsave);
    
    count = csmsave_read_ulong(csmsave);
    array = csmArrULong_new(count);
    
    for (i = 0; i < count; i++)
    {
        unsigned long element;
        
        element = csmsave_read_ulong(csmsave);
        csmArrULong_set(array, i, element);
    }
    
    return array;
}

// ----------------------------------------------------------------------------------------------------

csmArrayStruct(csmsave_item_t) *csmsave_dontuse_read_arr_st(
                        struct csmsave_t *csmsave,
                        const char *type_name,
                        csmsave_FPtr_read_struct func_read_struct)
{
    csmArrayStruct(csmsave_item_t) *array;
    unsigned long i, count;
    
    assert_no_null(csmsave);
    assert_no_null(func_read_struct);
    
    count = csmsave_read_ulong(csmsave);
    array = csmarrayc_new_st_array(count, csmsave_item_t);
    
    for (i = 0; i < count; i++)
    {
        struct csmsave_item_t *element;
        
        element = csmsave_dontuse_read_st(csmsave, type_name, func_read_struct);
        csmarrayc_set_st(array, i, element, csmsave_item_t);
    }
    
    return array;
}

// ----------------------------------------------------------------------------------------------------

struct csmsave_item_t *csmsave_dontuse_read_st(
                        struct csmsave_t *csmsave,
                        const char *type_name,
                        csmsave_FPtr_read_struct func_read_struct)
{
    assert_no_null(csmsave);
    assert_no_null(func_read_struct);
    
    if (csmsave->func_read_st_mark != NULL)
    {
        char *readed_string;
        
        readed_string = csmsave->func_read_st_mark(csmsave->csmsave_item);
        assert(csmstring_equal_strings(readed_string, type_name) == CSMTRUE);
        
        csmstring_free(&readed_string);
    }
    
    return func_read_struct(csmsave);
}

// ----------------------------------------------------------------------------------------------------

struct csmsave_item_t *csmsave_dontuse_read_optional_st(
                        struct csmsave_t *csmsave,
                        const char *type_name,
                        csmsave_FPtr_read_struct func_read_struct)
{
    assert_no_null(func_read_struct);
    
    if (csmsave_read_bool(csmsave) == CSMTRUE)
        return csmsave_dontuse_read_st(csmsave, type_name, func_read_struct);
    else
        return NULL;
}

