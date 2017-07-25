//
//  cypestr.h
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

CYBOOL cad_cadenas_iguales(const char *cadena1, const char *cadena2);

char *cad_copia_cadena(const char *cadena);

void cypestr_destruye(char **cadena);

#ifdef __cplusplus
}
#endif
