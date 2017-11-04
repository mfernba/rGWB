//
//  csmarrayc.c
//  cyscngraf
//
//  Created by Manuel Fernández on 15/02/14.
//  Copyright (c) 2014 Manuel Fernández. All rights reserved.
//

#include "csmarrayc.inl"

#include "csmassert.inl"
#include "csmarqsort.inl"
#include "csmmem.inl"
#include <string.h>

static const char i_DEBUG_MASK = 0xFA;
static const size_t i_INITIAL_CAPACITY = 100;

struct csmarrayc_t
{
    char mascara_debug;
    
    CSMBOOL is_pointer_array;
    size_t num_elementos;
    size_t capacidad;
    
    void *ptr_datos;
    size_t tamanyo_tipo_dato;
};

struct i_cmp_data_t
{
    struct csmarrayc_t *array;
    csmarrayc_FPtr_compare func_compare;
};

struct i_cmp_data_1_extra_t
{
    struct csmarrayc_t *array;
    struct csmarrayc_extra_item_t *extra_item;
    csmarrayc_FPtr_compare_1_extra func_compare;
};

// ---------------------------------------------------------------------------------

static void i_integrity(const struct csmarrayc_t *array)
{
    assert_no_null(array);
    assert(array->num_elementos <= array->capacidad);
    assert_no_null(array->ptr_datos);
    assert(array->tamanyo_tipo_dato > 0);
    assert(array->mascara_debug == i_DEBUG_MASK);
}

// ---------------------------------------------------------------------------------

static struct csmarrayc_t *i_crea(
                                  unsigned short is_pointer_array,
                                  size_t num_elementos,
                                  size_t capacidad,
                                  void **ptr_datos,
                                  size_t tamanyo_tipo_dato)
{
    struct csmarrayc_t *array;
    
    array = MALLOC(struct csmarrayc_t);
    
    array->is_pointer_array = is_pointer_array;
    
    array->num_elementos = num_elementos;
    array->capacidad = capacidad;
    array->ptr_datos = ASIGNA_PUNTERO_PP_NO_NULL(ptr_datos, void);
    array->tamanyo_tipo_dato = tamanyo_tipo_dato;
    array->mascara_debug = i_DEBUG_MASK;
    
    i_integrity(array);
    
    return array;
}

// ---------------------------------------------------------------------------------

struct csmarrayc_t *csmarrayc_dontuse_new_ptr_array(size_t capacidad_inicial, size_t tamanyo_tipo_dato)
{
    CSMBOOL is_pointer_array;
    size_t num_elementos;
    size_t capacidad;
    void *ptr_datos;
    
    is_pointer_array = CSMTRUE;
    
    if (capacidad_inicial == 0)
    {
        num_elementos = 0;
        capacidad = i_INITIAL_CAPACITY;
    }
    else
    {
        num_elementos = capacidad_inicial;
        capacidad = capacidad_inicial;
    }
    
    ptr_datos = (void *)malloc(tamanyo_tipo_dato * capacidad);
    
    return i_crea(is_pointer_array, num_elementos, capacidad, &ptr_datos, tamanyo_tipo_dato);
}

// ---------------------------------------------------------------------------------

void csmarrayc_nousar_destruye(struct csmarrayc_t **array, csmarrayc_FPtr_free_struct func_free_struct)
{
    assert_no_null(array);
    i_integrity(*array);
    
    if ((*array)->is_pointer_array == CSMTRUE && func_free_struct != NULL)
    {
        void *ptr_datos;
        unsigned long i, offset;
        
        ptr_datos = (*array)->ptr_datos;
        offset = 0;
        
        for (i = 0; i < (*array)->num_elementos; i++)
        {
            func_free_struct(((void **)(ptr_datos + offset)));
            offset += (*array)->tamanyo_tipo_dato;
        }
    }

    FREE_PP(&(*array)->ptr_datos, void);
    
    FREE_PP(array, struct csmarrayc_t);
}

// ---------------------------------------------------------------------------------

size_t csmarrayc_nousar_num_elems(const struct csmarrayc_t *array)
{
    i_integrity(array);
    return array->num_elementos;
}

// ---------------------------------------------------------------------------------

void csmarrayc_nousar_append_elemento(struct csmarrayc_t *array, void *dato)
{
    i_integrity(array);
    
    if (array->capacidad == array->num_elementos)
    {
        size_t nueva_capacidad;
        void *ptr_datos_ampliado;
        
        nueva_capacidad = array->capacidad + (3 * array->capacidad) / 2;
        ptr_datos_ampliado = (void *)malloc(nueva_capacidad * array->tamanyo_tipo_dato);
        assert_no_null(ptr_datos_ampliado);
        
        //memset(ptr_datos_ampliado, 0xFF, nueva_capacidad * array->tamanyo_tipo_dato);
        memcpy(ptr_datos_ampliado, array->ptr_datos, array->num_elementos * array->tamanyo_tipo_dato);
        
        FREE_PP(&array->ptr_datos, void);
        
        array->ptr_datos = ASIGNA_PUNTERO_PP_NO_NULL(&ptr_datos_ampliado, void);
        array->capacidad = nueva_capacidad;;
    }
    
    memcpy(array->ptr_datos + array->num_elementos * array->tamanyo_tipo_dato, dato, array->tamanyo_tipo_dato);
    array->num_elementos++;
}

// ---------------------------------------------------------------------------------

void csmarrayc_nousar_set_element(struct csmarrayc_t *array, unsigned long idx, void *dato)
{
    i_integrity(array);
    assert(idx < array->num_elementos);
    
    memcpy(array->ptr_datos + idx * array->tamanyo_tipo_dato, dato, array->tamanyo_tipo_dato);
}

// ---------------------------------------------------------------------------------

CSMBOOL csmarrayc_nousar_contains_element(
                        const struct csmarrayc_t *array,
                        const csmarrayc_byte *search_data,
                        csmarrayc_FPtr_match_condition func_match_condition,
                        unsigned long *idx)
{
    CSMBOOL exists_element;
    unsigned long idx_loc;
    unsigned long i;
    unsigned long offset;
    
    assert_no_null(array);
    
    exists_element = CSMFALSE;
    idx_loc = ULONG_MAX;
    
    offset = 0;
    
    for (i = 0; i < array->num_elementos; i++)
    {
        const void *element;
        
        element = *(void **)(array->ptr_datos + offset);
        
        if (func_match_condition(element, search_data) == CSMTRUE)
        {
            exists_element = CSMTRUE;
            idx_loc = i;
            break;
        }
        
        offset += array->tamanyo_tipo_dato;
    }
    
    ASSIGN_OPTIONAL_VALUE(idx, idx_loc);
    
    return exists_element;
}

// ---------------------------------------------------------------------------------

void *csmarrayc_nousar_get(struct csmarrayc_t *array, unsigned long idx)
{
    unsigned long offset;
    
    assert_no_null(array);
    assert(idx < array->num_elementos);
    
    offset = array->tamanyo_tipo_dato * idx;
    return (void *)(*(void **)(array->ptr_datos + offset));
}

// ---------------------------------------------------------------------------------

void csmarrayc_nousar_delete_element(struct csmarrayc_t *array, unsigned long idx, csmarrayc_FPtr_free_struct func_free)
{
    unsigned long offset;
    
    assert_no_null(array);
    assert(idx < array->num_elementos);
    
    offset = array->tamanyo_tipo_dato * idx;
    
    if (func_free != NULL)
        func_free((void **)(array->ptr_datos + offset));
    
    if (idx < array->num_elementos - 1)
    {
        size_t bytes_a_mover;
        
        bytes_a_mover = (array->num_elementos - 1 - idx) * array->tamanyo_tipo_dato;
        memmove(array->ptr_datos + offset, array->ptr_datos + offset + array->tamanyo_tipo_dato, bytes_a_mover);
    }
    
    array->num_elementos--;
}

// ---------------------------------------------------------------------------------

static int i_cmp_function_ptr(const void *cmp_data_void, const void *e1, const void *e2)
{
    struct i_cmp_data_t *cmp_data;
    
    cmp_data = (struct i_cmp_data_t *)cmp_data_void;
    assert_no_null(cmp_data);
    
    return (int)cmp_data->func_compare(*(void **)e1, *(void **)e2);
}

// ---------------------------------------------------------------------------------

void csmarrayc_nousar_qsort(struct csmarrayc_t *array, csmarrayc_FPtr_compare func_compare)
{
    struct i_cmp_data_t cmp_data;
    
    assert_no_null(array);
    
    cmp_data.array = array;
    cmp_data.func_compare = func_compare;
    
    csmarqsort(array->ptr_datos, array->num_elementos, array->tamanyo_tipo_dato, (void *)&cmp_data, i_cmp_function_ptr);
}

// ---------------------------------------------------------------------------------

static int i_cmp_function_ptr_1_extra(const void *cmp_data_void, const void *e1, const void *e2)
{
    struct i_cmp_data_1_extra_t *cmp_data;
    
    cmp_data = (struct i_cmp_data_1_extra_t *)cmp_data_void;
    assert_no_null(cmp_data);
    
    return (int)cmp_data->func_compare(*(void **)e1, *(void **)e2, cmp_data->extra_item);
}

// ---------------------------------------------------------------------------------

void csmarrayc_nousar_qsort_1_extra(
                        struct csmarrayc_t *array,
                        struct csmarrayc_extra_item_t *extra_item,
                        csmarrayc_FPtr_compare_1_extra func_compare_1_extra)
{
    struct i_cmp_data_1_extra_t cmp_data;
    
    assert_no_null(array);
    
    cmp_data.array = array;
    cmp_data.extra_item = extra_item;
    cmp_data.func_compare = func_compare_1_extra;
    
    csmarqsort(array->ptr_datos, array->num_elementos, array->tamanyo_tipo_dato, (void *)&cmp_data, i_cmp_function_ptr_1_extra);
}












