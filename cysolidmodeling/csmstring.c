//
//  cypestr.c
//  rGWB
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmstring.inl"

#include "csmassert.inl"
#include "csmmem.inl"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// ------------------------------------------------------------------------------------------

CSMBOOL csmstring_equal_strings(const char *cadena1, const char *cadena2)
{
    if (strcmp(cadena1, cadena2) == 0)
        return CSMTRUE;
    else
        return CSMFALSE;
}

// ------------------------------------------------------------------------------------------

char *csmstring_duplicate(const char *cadena)
{
    return strdup(cadena);
}

// ------------------------------------------------------------------------------------------

void csmstring_free(char **cadena)
{
    FREE_PP(cadena, char);
}

// --------------------------------------------------------------------------------

char *copiafor_nousar(const char *formato, ...)
{
    int resultado;
    char buffer[10000];
    va_list argptr;
    
    va_start(argptr, formato);
    resultado = vsnprintf(buffer, sizeof(buffer) - 1, formato, argptr);
    assert(resultado >= 0);
    buffer[sizeof(buffer) - 1] = '\0';
    va_end(argptr);
    
    return csmstring_duplicate(buffer);
}
