//
//  csmsave.c
//  rGWB
//
//  Created by Manuel Fernández on 18/6/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmsave.inl"

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

void csmsave_write_uchar(struct csmsave_t *csmsave, unsigned char value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);
    
    fprintf(csmsave->file_descriptor, "%lu ", value);
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

void csmsave_write_char(struct csmsave_t *csmsave, const char *value)
{
    assert_no_null(csmsave);
    assert(csmsave->mode == i_MODE_WRITE);

    fprintf(csmsave->file_descriptor, "%s ", value);
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
