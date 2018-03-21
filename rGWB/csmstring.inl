//
//  csmstring.inl
//  rGWB
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

CSMBOOL csmstring_equal_strings(const char *cadena1, const char *cadena2);

char *csmstring_duplicate(const char *cadena);

void csmstring_free(char **cadena);
    

#ifdef __STANDALONE_DISTRIBUTABLE
    
char *copiafor_nousar(const char *formato, ...);
#define copiafor_codigo0(formato) copiafor_nousar(formato)
#define copiafor_codigo1(formato, parametro1) copiafor_nousar(formato, parametro1)
#define copiafor_codigo2(formato, parametro1, parametro2) copiafor_nousar(formato, parametro1, parametro2)
#define copiafor_codigo3(formato, parametro1, parametro2, parametro3) copiafor_nousar(formato, parametro1, parametro2, parametro3)
#define copiafor_codigo4(formato, parametro1, parametro2, parametro3, parametro4) copiafor_nousar(formato, parametro1, parametro2, parametro3, parametro4)
#define copiafor_codigo5(formato, parametro1, parametro2, parametro3, parametro4, parametro5) copiafor_nousar(formato, parametro1, parametro2, parametro3, parametro4, parametro5)
#define copiafor_codigo6(formato, parametro1, parametro2, parametro3, parametro4, parametro5, parametro6) copiafor_nousar(formato, parametro1, parametro2, parametro3, parametro4, parametro5, parametro6)
    
#endif

#ifdef __cplusplus
}
#endif
