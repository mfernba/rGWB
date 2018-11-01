//
//  csmsavetxt.c
//  rGWB
//
//  Created by Manuel Fernández on 1/11/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmsavetxt.h"

#include "csmarrayc.h"
#include "csmaulong.h"
#include "csmsave.inl"
#include "csmsave.h"
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

struct csmsavetxt_t
{
    FILE *file_descriptor;
    enum i_mode_t mode;
};

#define i_STRING_BUFFER_SIZE 1024 * 1024

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsavetxt_t *, i_new, (FILE **file_descriptor, enum i_mode_t mode))
{
    struct csmsavetxt_t *csmsave;
    
    csmsave = MALLOC(struct csmsavetxt_t);
    
    csmsave->file_descriptor = ASSIGN_POINTER_PP_NOT_NULL(file_descriptor, FILE);
    csmsave->mode = mode;
    
    return csmsave;
}

// ----------------------------------------------------------------------------------------------------

static void i_free(struct csmsavetxt_t **csmsave)
{
    assert_no_null(csmsave);
    assert_no_null(*csmsave);
    assert_no_null((*csmsave)->file_descriptor);
    
    fflush((*csmsave)->file_descriptor);
    fclose((*csmsave)->file_descriptor);
    (*csmsave)->file_descriptor = NULL;
    
    FREE_PP(csmsave, struct csmsavetxt_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_bool(const struct csmsavetxt_t *csmsave, CSMBOOL value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%hu\n", value);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_ushort(const struct csmsavetxt_t *csmsave, unsigned short value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%hu\n", value);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_uchar(const struct csmsavetxt_t *csmsave, unsigned char value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%hhu\n", value);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_ulong(const struct csmsavetxt_t *csmsave, unsigned long value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%lu\n", value);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_double(const struct csmsavetxt_t *csmsave, double value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);

    fprintf(csmsave->file_descriptor, "%.17lf\n", value);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_float(const struct csmsavetxt_t *csmsave, float value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);

    fprintf(csmsave->file_descriptor, "%.9f\n", value);
}

// ----------------------------------------------------------------------------------------------------

static void i_write_string(const struct csmsavetxt_t *csmsave, const char *value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);

    fprintf(csmsave->file_descriptor, "%s\n", value);
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_read_bool(const struct csmsavetxt_t *csmsave)
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

static unsigned char i_read_uchar(const struct csmsavetxt_t *csmsave)
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

static unsigned short i_read_ushort(const struct csmsavetxt_t *csmsave)
{
    unsigned short value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%hu\n", &value);
    assert(readed == 1);
    
    return (CSMBOOL)value;
}

// ----------------------------------------------------------------------------------------------------

static unsigned long i_read_ulong(const struct csmsavetxt_t *csmsave)
{
    unsigned long value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%lu\n", &value);
    assert(readed == 1);
    
    return value;
}

// ----------------------------------------------------------------------------------------------------

static double i_read_double(const struct csmsavetxt_t *csmsave)
{
    double value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%lf\n", &value);
    assert(readed == 1);
    
    return value;
}

// ----------------------------------------------------------------------------------------------------

static float i_read_float(const struct csmsavetxt_t *csmsave)
{
    float value;
    int readed;
    
    assert_no_null(csmsave);

    readed = fscanf(csmsave->file_descriptor, "%f\n", &value);
    assert(readed == 1);
    
    return value;
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static char *, i_read_string, (const struct csmsavetxt_t *csmsave))
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

CONSTRUCTOR(static struct csmsave_t *, i_new_csmsave, (struct csmsavetxt_t **csmsavetxt))
{
    return csmsave_new(
                    csmsavetxt, csmsavetxt_t,
                    i_free,
                    i_write_bool,
                    i_write_uchar,
                    i_write_ushort,
                    i_write_ulong,
                    i_write_double,
                    i_write_float,
                    i_write_string,
                    i_read_bool,
                    i_read_uchar,
                    i_read_ushort,
                    i_read_ulong,
                    i_read_double,
                    i_read_float,
                    i_read_string,
                    i_write_string,
                    i_read_string);
}

// ----------------------------------------------------------------------------------------------------

struct csmsave_t *csmsavetxt_new_file_writer(const char *file_path)
{
    FILE *file_descriptor;
    enum i_mode_t mode;
    struct csmsavetxt_t *csmsavetxt;
    
    file_descriptor = fopen(file_path, "wt");
    mode = i_MODE_WRITE;
    
    csmsavetxt = i_new(&file_descriptor, mode);
    return i_new_csmsave(&csmsavetxt);
}

// ----------------------------------------------------------------------------------------------------

struct csmsave_t *csmsavetxt_new_file_reader(const char *file_path)
{
    FILE *file_descriptor;
    enum i_mode_t mode;
    struct csmsavetxt_t *csmsavetxt;
    
    file_descriptor = fopen(file_path, "rt");
    mode = i_MODE_READ;
    
    csmsavetxt = i_new(&file_descriptor, mode);
    return i_new_csmsave(&csmsavetxt);
}
