//
//  csgarrayc.c
//  cyscngraf
//
//  Created by Manuel Fernández on 15/02/14.
//  Copyright (c) 2014 Manuel Fernández. All rights reserved.
//

#include "csmarrayc.inl"

#include "csmassert.inl"
#include "csmmem.inl"
#include <string.h>

static const char i_DEBUG_MASK = 0xFA;
static const size_t i_CAPACITY_INCR = 100;

struct csgarrayc_t
{
    size_t num_elementos;
    size_t capacidad;
    
    csgarrayc_byte *ptr_datos;
    size_t tamanyo_tipo_dato;
    
    char mascara_debug;
};

// ---------------------------------------------------------------------------------

static void i_integrity(const struct csgarrayc_t *array)
{
    assert_no_null(array);
    assert(array->num_elementos <= array->capacidad);
    assert_no_null(array->ptr_datos);
    assert(array->tamanyo_tipo_dato > 0);
    assert(array->mascara_debug == i_DEBUG_MASK);
}

// ---------------------------------------------------------------------------------

static struct csgarrayc_t *i_crea(
                                     size_t num_elementos,
                                     size_t capacidad,
                                     csgarrayc_byte **ptr_datos,
                                     size_t tamanyo_tipo_dato)
{
    struct csgarrayc_t *array;
    
    array = MALLOC(struct csgarrayc_t);
    
    array->num_elementos = num_elementos;
    array->capacidad = capacidad;
    array->ptr_datos = ASIGNA_PUNTERO_PP_NO_NULL(ptr_datos, csgarrayc_byte);
    array->tamanyo_tipo_dato = tamanyo_tipo_dato;
    array->mascara_debug = i_DEBUG_MASK;
    
    i_integrity(array);
    
    return array;
}

// ---------------------------------------------------------------------------------

struct csgarrayc_t *csgarrayc_nousar_crea(size_t capacidad_inicial, size_t tamanyo_tipo_dato)
{
    size_t num_elementos;
    size_t capacidad;
    csgarrayc_byte *ptr_datos;
    
    num_elementos = 0;
    
    if (capacidad_inicial == 0)
        capacidad = i_CAPACITY_INCR;
    else
        capacidad = capacidad_inicial;
    
    ptr_datos = CALLOC(tamanyo_tipo_dato * capacidad, csgarrayc_byte);
    
    return i_crea(num_elementos, capacidad, &ptr_datos, tamanyo_tipo_dato);
}

// ---------------------------------------------------------------------------------

void csgarrayc_nousar_destruye(struct csgarrayc_t **array)
{
    assert_no_null(array);
    i_integrity(*array);
    
    FREE_PP(&(*array)->ptr_datos, csgarrayc_byte);
    
    FREE_PP(array, struct csgarrayc_t);
}

// ---------------------------------------------------------------------------------

size_t csgarrayc_nousar_num_elems(const struct csgarrayc_t *array)
{
    i_integrity(array);
    return array->num_elementos;
}

// ---------------------------------------------------------------------------------

void csgarrayc_nousar_append_elemento(struct csgarrayc_t *array, csgarrayc_byte *dato)
{
    size_t indice_base_desplazamiento;
    
    i_integrity(array);
    
    if (array->capacidad == array->num_elementos)
    {
        size_t nueva_capacidad;
        csgarrayc_byte *ptr_datos_ampliado;
        
        nueva_capacidad = array->capacidad + i_CAPACITY_INCR;
        ptr_datos_ampliado = (csgarrayc_byte *)malloc(nueva_capacidad * array->tamanyo_tipo_dato);
        assert_no_null(ptr_datos_ampliado);
        
        //memset(ptr_datos_ampliado, 0xFF, nueva_capacidad * array->tamanyo_tipo_dato);
        memcpy(ptr_datos_ampliado, array->ptr_datos, array->num_elementos * array->tamanyo_tipo_dato);
        
        FREE_PP(&array->ptr_datos, csgarrayc_byte);
        
        array->ptr_datos = ASIGNA_PUNTERO_PP_NO_NULL(&ptr_datos_ampliado, csgarrayc_byte);
        array->capacidad = nueva_capacidad;;
    }
    
    indice_base_desplazamiento = array->num_elementos * array->tamanyo_tipo_dato;
    memcpy(&array->ptr_datos[indice_base_desplazamiento], dato, array->tamanyo_tipo_dato);
    
    array->num_elementos++;
}

// ---------------------------------------------------------------------------------

csgarrayc_byte *csgarrayc_nousar_dame_ptr_datos_y_num_elementos(struct csgarrayc_t *array, size_t *num_elementos_opc)
{
    i_integrity(array);
    
    ASSIGN_OPTIONAL_VALUE(num_elementos_opc, array->num_elementos);
    return array->ptr_datos;
}
















