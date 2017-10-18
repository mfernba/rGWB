//
//  csmarrayc.h
//  cyscngraf
//
//  Created by Manuel Fernández on 15/02/14.
//  Copyright (c) 2014 Manuel Fernández. All rights reserved.
//

#include "csmarrayc.hxx"
#include "csmfwddecl.hxx"

struct csmarrayc_t *csmarrayc_dontuse_new_ptr_array(size_t capacidad_inicial, size_t tamanyo_tipo_dato);

void csmarrayc_nousar_destruye(struct csmarrayc_t **array, csmarrayc_FPtr_free_struct func_free_struct);

size_t csmarrayc_nousar_num_elems(const struct csmarrayc_t *array);

void csmarrayc_nousar_append_elemento(struct csmarrayc_t *array, void *dato);

void csmarrayc_nousar_set_element(struct csmarrayc_t *array, unsigned long idx, void *dato);

CSMBOOL csmarrayc_nousar_contains_element(
                        const struct csmarrayc_t *array,
                        const csmarrayc_byte *search_data,
                        csmarrayc_FPtr_match_condition func_match_condition,
                        unsigned long *idx_opt);

void *csmarrayc_nousar_get(struct csmarrayc_t *array, unsigned long idx);

void csmarrayc_nousar_delete_element(struct csmarrayc_t *array, unsigned long idx, csmarrayc_FPtr_free_struct func_free);

void csmarrayc_nousar_qsort(struct csmarrayc_t *array, csmarrayc_FPtr_compare func_compare);


// Array of pointers to structs...

#define csmarrayc_new_st_array(capacidad_inicial, tipo) (csmArrayStruct(tipo) *)csmarrayc_dontuse_new_ptr_array(capacidad_inicial, sizeof(struct tipo *))
#define csmarrayc_new_const_st_array(capacidad_inicial, tipo) (const csmArrayStruct(tipo) *)csmarrayc_dontuse_new_ptr_array(capacidad_inicial, sizeof(struct tipo *))

#define csmarrayc_free_st(array, tipo, func_free)\
(\
    (void)((csmArrayStruct(tipo) **)array == array),\
    CSMARRAYC_CHECK_FUNC_FREE_STRUCT(func_free, tipo),\
    csmarrayc_nousar_destruye((struct csmarrayc_t **)array, (csmarrayc_FPtr_free_struct)func_free)\
)

#define csmarrayc_free_const_st(array, tipo)\
(\
    (void)((const csmArrayStruct(tipo) **)array == array),\
    csmarrayc_nousar_destruye((struct csmarrayc_t **)array, NULL)\
)

#define csmarrayc_count_st(array, tipo)\
(\
    (void)((csmArrayStruct(tipo) *)array == array),\
    csmarrayc_nousar_num_elems((const struct csmarrayc_t *)array)\
)

#define csmarrayc_append_element_st(array, dato, tipo)\
(\
    (void)((csmArrayStruct(tipo) *)array == array),\
    (void)((struct tipo *)dato == dato),\
    (void)csmarrayc_nousar_append_elemento((struct csmarrayc_t *)array, (void *)(&(dato)))\
)

#define csmarrayc_append_element_const_st(array, dato, tipo)\
(\
    (void)((const csmArrayStruct(tipo) *)array == array),\
    (void)((const struct tipo *)dato == dato),\
    (void)csmarrayc_nousar_append_elemento((struct csmarrayc_t *)array, (void *)(&(dato)))\
)

#define csmarrayc_contains_element_st(array, array_type, search_data, search_data_type, func_match_condition, idx_opt)\
(\
    (void)((csmArrayStruct(array_type) *)array == array),\
    (void)((search_data_type *)search_data == search_data),\
    CSMARRAYC_CHECK_FUNC_MATCH_CONDITION(func_match_condition, array_type, search_data_type),\
    csmarrayc_nousar_contains_element((struct csmarrayc_t *)array, (const void *)search_data, (csmarrayc_FPtr_match_condition)func_match_condition, idx_opt)\
)

#define csmarrayc_get_st(array, idx, tipo)\
(\
    (void)((csmArrayStruct(tipo) *)array == array),\
    (struct tipo *)csmarrayc_nousar_get((struct csmarrayc_t *)array, idx)\
)

#define csmarrayc_get_const_st(array, idx, tipo)\
(\
    (void)((const csmArrayStruct(tipo) *)array == array),\
    (const struct tipo *)csmarrayc_nousar_get((struct csmarrayc_t *)array, idx)\
)

#define csmarrayc_set_st(array, idx, element, tipo)\
(\
    (void)((csmArrayStruct(tipo) *)array == array),\
    (void)((struct tipo *)element == element),\
    csmarrayc_nousar_set_element((struct csmarrayc_t *)array, idx, (void *)(&(element)))\
)

#define csmarrayc_set_const_st(array, idx, element, tipo)\
(\
    (void)((const csmArrayStruct(tipo) *)array == array),\
    (void)((struct tipo *)element == element),\
    csmarrayc_nousar_set_element((struct csmarrayc_t *)array, idx, (void *)(&(element)))\
)

#define csmarrayc_delete_element_st(array, idx, tipo, func_free)\
(\
    (void)((csmArrayStruct(tipo) *)array == array),\
    CSMARRAYC_CHECK_FUNC_FREE_STRUCT(func_free, tipo),\
    csmarrayc_nousar_delete_element((struct csmarrayc_t *)array, idx, (csmarrayc_FPtr_free_struct)func_free)\
)

#define csmarrayc_qsort_st(array, tipo, func_compare)\
(\
    (void)((csmArrayStruct(tipo) *)array == array),\
    CSMARRAYC_CHECK_FUNC_COMPARE_ST(func_compare, tipo),\
    csmarrayc_nousar_qsort((struct csmarrayc_t *)array, (csmarrayc_FPtr_compare)func_compare)\
)

