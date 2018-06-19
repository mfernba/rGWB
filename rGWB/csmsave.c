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
    
    fclose((*csmsave)->file_descriptor);
    (*csmsave)->file_descriptor = NULL;
    
    FREE_PP(csmsave, struct csmsave_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_block_separator(struct csmsave_t *csmsave)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "\n");
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_bool(struct csmsave_t *csmsave, CSMBOOL value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%hu ", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_uchar(struct csmsave_t *csmsave, unsigned char value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%hhu ", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_ulong(struct csmsave_t *csmsave, unsigned long value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%lu ", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_double(struct csmsave_t *csmsave, double value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);

    fprintf(csmsave->file_descriptor, "%lf ", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_float(struct csmsave_t *csmsave, float value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);

    fprintf(csmsave->file_descriptor, "%f ", value);
}

// ----------------------------------------------------------------------------------------------------

void csmsave_write_char(struct csmsave_t *csmsave, const char *value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);

    fprintf(csmsave->file_descriptor, "%s ", value);
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
                        const csmArrayStruct(csmsave_item_t) *array,
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
        func_write_struct(element, csmsave);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmsave_dontuse_write_optional_st(
                        struct csmsave_t *csmsave,
                        const struct csmsave_item_t *item,
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
        func_write_struct(item, csmsave);
    }
}

// ----------------------------------------------------------------------------------------------------

void csmsave_read_block_separator(struct csmsave_t *csmsave)
{
    int readed;
    char block_separator[1024];
    
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_READ);
    
    readed = fscanf(csmsave->file_descriptor, "%s", block_separator);
    assert(readed == 1);
    
    assert(block_separator[0] == '\n');
}

// ----------------------------------------------------------------------------------------------------

unsigned char csmsave_read_uchar(struct csmsave_t *csmsave)
{
    unsigned long value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%lu", &value);
    assert(readed == 1);
    assert(value < 255);
    
    return (unsigned char)value;
}

// ----------------------------------------------------------------------------------------------------

unsigned long csmsave_read_ulong(struct csmsave_t *csmsave)
{
    unsigned long value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%lu", &value);
    assert(readed == 1);
    
    return value;
}

// ----------------------------------------------------------------------------------------------------

double csmsave_read_double(struct csmsave_t *csmsave)
{
    double value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%lf", &value);
    assert(readed == 1);
    
    return value;
}

// ----------------------------------------------------------------------------------------------------

char *csmsave_read_char(struct csmsave_t *csmsave)
{
    int readed;
    char value[1024 * 1024];
    
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_READ);
    
    readed = fscanf(csmsave->file_descriptor, "%s", value);
    assert(readed == 1);
    
    return csmstring_duplicate(value);
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
        
        element = func_read_struct(csmsave);
        csmarrayc_set_st(array, i, element, csmsave_item_t);
    }
    
    return array;
}
