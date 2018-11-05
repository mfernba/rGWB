//
//  cypestr.c
//  rGWB
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmstring.inl"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#else
#include "cypespy.h"
#include "cypestr.h"
#endif

// ------------------------------------------------------------------------------------------

CSMBOOL csmstring_equal_strings(const char *cadena1, const char *cadena2)
{
    #ifdef RGWB_STANDALONE_DISTRIBUTABLE
    if (strcmp(cadena1, cadena2) == 0)
    #else
    if (cad_cadenas_iguales(cadena1, cadena2) == CIERTO)
    #endif
        return CSMTRUE;
    else
        return CSMFALSE;
}

// ------------------------------------------------------------------------------------------

char *csmstring_duplicate(const char *cadena)
{
    #ifdef RGWB_STANDALONE_DISTRIBUTABLE
        return strdup(cadena);
    #else
        return cad_copia_cadena(cadena);
    #endif
}

// ------------------------------------------------------------------------------------------

void csmstring_free(char **cadena)
{
    FREE_PP(cadena, char);
}

// ------------------------------------------------------------------------------------------

#ifdef RGWB_STANDALONE_DISTRIBUTABLE

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

#endif
