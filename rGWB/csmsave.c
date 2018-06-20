//
//  csmsave.c
//  rGWB
//
//  Created by Manuel Fernández on 18/6/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmsave.inl"

#include "csmarrayc.h"
#include "csmArrULong.h"
#include "csmstring.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

enum i_mode_t
{
    i_MODE_WRITE,
    i_MODE_READ
};

struct csmsave_t
{
    FILE *file_descriptor;
    enum i_mode_t mode;
};

#define i_STRING_BUFFER_SIZE 1024 * 1024

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsave_t *, i_new, (FILE **file_descriptor, enum i_mode_t mode))
{
    struct csmsave_t *csmsave;
    
    csmsave = MALLOC(struct csmsave_t);
    
    csmsave->file_descriptor = ASSIGN_POINTER_PP_NOT_NULL(file_descriptor, FILE);
    csmsave->mode = mode;
    
    return csmsave;
}

// ----------------------------------------------------------------------------------------------------

struct csmsave_t *csmsave_new_file_writer(const char *file_path)
{
    FILE *file_descriptor;
    enum i_mode_t mode;
    
    file_descriptor = fopen(file_path, "wt");
    mode = i_MODE_WRITE;
    
    return i_new(&file_descriptor, mode);
}

// ----------------------------------------------------------------------------------------------------

struct csmsave_t *csmsave_new_file_reader(const char *file_path)
{
    FILE *file_descriptor;
    enum i_mode_t mode;
    
    file_descriptor = fopen(file_path, "rt");
    mode = i_MODE_READ;
    
    return i_new(&file_descriptor, mode);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_free(struct csmsave_t **csmsave)
{
    assert_no_null(csmsave);
    assert_no_null(*csmsave);
    assert_no_null((*csmsave)->file_descriptor);
    
    fflush((*csmsave)->file_descriptor);
    fclose((*csmsave)->file_descriptor);
    (*csmsave)->file_descriptor = NULL;
    
    FREE_PP(csmsave, struct csmsave_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_bool(struct csmsave_t *csmsave, CSMBOOL value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%hu\n", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_ushort(struct csmsave_t *csmsave, unsigned short value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%hu\n", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_uchar(struct csmsave_t *csmsave, unsigned char value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%hhu\n", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_ulong(struct csmsave_t *csmsave, unsigned long value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%lu\n", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_double(struct csmsave_t *csmsave, double value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);

    fprintf(csmsave->file_descriptor, "%.17lf\n", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_float(struct csmsave_t *csmsave, float value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);

    fprintf(csmsave->file_descriptor, "%.9f\n", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_string(struct csmsave_t *csmsave, const char *value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);

    fprintf(csmsave->file_descriptor, "%s\n", value);
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
    assert(csmsave->mode == i_MODE_WRITE);

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
    assert(csmsave->mode == i_MODE_WRITE);
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
    
    fprintf(csmsave->file_descriptor, "%s\n", type_name);
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
    unsigned short value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%hu\n", &value);
    assert(readed == 1);
    assert(value == 0 || value == 1);
    
    return (CSMBOOL)value;
}

// ----------------------------------------------------------------------------------------------------

unsigned char csmsave_read_uchar(struct csmsave_t *csmsave)
{
    unsigned char value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%hhu", &value);
    assert(readed == 1);
    assert(value < 255);
    
    return (unsigned char)value;
}

// ----------------------------------------------------------------------------------------------------

unsigned short csmsave_read_ushort(struct csmsave_t *csmsave)
{
    unsigned short value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%hu\n", &value);
    assert(readed == 1);
    
    return (CSMBOOL)value;
}

// ----------------------------------------------------------------------------------------------------

unsigned long csmsave_read_ulong(struct csmsave_t *csmsave)
{
    unsigned long value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%lu\n", &value);
    assert(readed == 1);
    
    return value;
}

// ----------------------------------------------------------------------------------------------------

double csmsave_read_double(struct csmsave_t *csmsave)
{
    double value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%lf\n", &value);
    assert(readed == 1);
    
    return value;
}

// ----------------------------------------------------------------------------------------------------

float csmsave_read_float(struct csmsave_t *csmsave)
{
    float value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%f\n", &value);
    assert(readed == 1);
    
    return value;
}

// ----------------------------------------------------------------------------------------------------

char *csmsave_read_string(struct csmsave_t *csmsave)
{
    int readed;
    char value[i_STRING_BUFFER_SIZE];
    
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_READ);
    
    readed = fscanf(csmsave->file_descriptor, "%s\n", value);
    assert(readed == 1);
    
    return csmstring_duplicate(value);
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
    assert(csmsave->mode == i_MODE_READ);
    
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
    assert(csmsave->mode == i_MODE_READ);
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
    char value[i_STRING_BUFFER_SIZE];
    int readed;
    
    assert_no_null(csmsave);
    assert_no_null(func_read_struct);
    assert(csmsave->mode == i_MODE_READ);
    
    readed = fscanf(csmsave->file_descriptor, "%s\n", value);
    assert(readed == 1);
    assert(csmstring_equal_strings(value, type_name) == CSMTRUE);
    
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

