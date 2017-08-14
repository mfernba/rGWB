//
//  sm4assert.c
//  solidmodeling4
//
//  Created by Manuel Fernandez on 1/2/15.
//  Copyright (c) 2015 manueru. All rights reserved.
//

#include "cyassert.h"

#include "csmdebug.inl"

#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------------

void cyassert_dontuse_assertion_failed(CYBOOL condition, const char *file, int line, const char *assertion)
{
    if (condition == FALSO)
    {
        csmdebug_close_output_file();
        
        fprintf(stderr, "Solid modeler assertion failed: (%s)\nFILE: %s\nLINE: %d\n", assertion, file, line);
        _Exit(-1);
    }
}

// ---------------------------------------------------------------------------------

void cyassert_dontuse_assertion_not_null_failed(const void *ptr, const char *file, int line, const char *assertion)
{
    cyassert_dontuse_assertion_failed(ptr != NULL, file, line, assertion);
}

// ---------------------------------------------------------------------------------

void cyassert_dontuse_default_error(const char *file, int line)
{
    fprintf(stderr, "Solid modeler switch case not handled at FILE: %s\nLINE: %d\n", file, line);
    _Exit(-1);
}
