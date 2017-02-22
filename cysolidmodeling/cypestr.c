//
//  cypestr.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "cypestr.h"

#include "cypespy.h"
#include <string.h>

// ------------------------------------------------------------------------------------------

CYBOOL cad_cadenas_iguales(const char *cadena1, const char *cadena2)
{
    if (strcmp(cadena1, cadena2) == 0)
        return CIERTO;
    else
        return FALSO;
}

// ------------------------------------------------------------------------------------------

char *cad_copia_cadena(const char *cadena)
{
    return strdup(cadena);
}

// ------------------------------------------------------------------------------------------

void cypestr_destruye(char **cadena)
{
    FREE_PP(cadena, char);
}
