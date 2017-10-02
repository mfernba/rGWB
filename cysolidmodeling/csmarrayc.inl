//
//  csgarrayc.h
//  cyscngraf
//
//  Created by Manuel Fernández on 15/02/14.
//  Copyright (c) 2014 Manuel Fernández. All rights reserved.
//

#ifndef cyscngraf_csmArrayST_h
#define cyscngraf_csmArrayST_h

#include "csmfwddecl.hxx"

#define csmArrayST(tipo) struct csmArrayST_##tipo
typedef unsigned char csgarrayc_byte;

struct csgarrayc_t *csgarrayc_nousar_crea(size_t capacidad_inicial, size_t tamanyo_tipo_dato);
#define csgarrayc_crea(capacidad_inicial, tipo) (csgarrayc(tipo) *)csgarrayc_nousar_crea(capacidad_inicial, sizeof(tipo))

void csgarrayc_nousar_destruye(struct csgarrayc_t **array);
#define csgarrayc_destruye(array, tipo)\
(\
    (void)((csgarrayc(tipo) **)array == array),\
    csgarrayc_nousar_destruye((struct csgarrayc_t **)array)\
)

size_t csgarrayc_nousar_num_elems(const struct csgarrayc_t *array);
#define csgarrayc_num_elems(array, tipo)\
(\
    (void)((csgarrayc(tipo) *)array == array),\
    csgarrayc_nousar_num_elems((const struct csgarrayc_t *)array)\
)

void csgarrayc_nousar_append_elemento(struct csgarrayc_t *array, csgarrayc_byte *dato);
#define csgarrayc_append_elemento(array, dato, tipo)\
(\
    (void)((csgarrayc(tipo) *)array == array),\
    (void)((tipo)dato == dato),\
    (void)csgarrayc_nousar_append_elemento((struct csgarrayc_t *)array, (csgarrayc_byte *)&dato)\
)

csgarrayc_byte *csgarrayc_nousar_dame_ptr_datos_y_num_elementos(struct csgarrayc_t *array, size_t *num_elementos_opc);
#define csgarrayc_dame_ptr_datos_y_num_elementos(array, num_elementos_opc, tipo)\
(\
    (void)((csgarrayc(tipo) *)array == array),\
    (tipo *)csgarrayc_nousar_dame_ptr_datos_y_num_elementos((struct csgarrayc_t *)array, num_elementos_opc)\
)

#endif
