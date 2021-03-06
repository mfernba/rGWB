// Generic container...

#include "csmarrayc.hxx"
#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB struct csmarrayc_t *csmarrayc_dontuse_new_ptr_array(unsigned long capacidad_inicial, size_t tamanyo_type_dato);

DLL_RGWB struct csmarrayc_t *csmarrayc_dontuse_copy_ptr_array(const struct csmarrayc_t *array, csmarrayc_FPtr_copy_struct func_copy_element);

DLL_RGWB void csmarrayc_dontuse_free(struct csmarrayc_t **array, csmarrayc_FPtr_free_struct func_free_struct);

DLL_RGWB unsigned long csmarrayc_dontuse_count(const struct csmarrayc_t *array);

DLL_RGWB void csmarrayc_dontuse_insert_element(struct csmarrayc_t *array, unsigned long pos, void *element);

DLL_RGWB void csmarrayc_dontuse_append_element(struct csmarrayc_t *array, void *element);

DLL_RGWB void csmarrayc_dontuse_set_element(struct csmarrayc_t *array, unsigned long idx, void *element);

DLL_RGWB void csmarrayc_dontuse_concat(
                       struct csmarrayc_t *array, const struct csmarrayc_t *array_origin,
                       csmarrayc_FPtr_copy_struct func_copy_element);

DLL_RGWB CSMBOOL csmarrayc_dontuse_contains_element(
                        const struct csmarrayc_t *array,
                        const csmarrayc_byte *search_data,
                        csmarrayc_FPtr_match_condition func_match_condition,
                        unsigned long *idx_opt);

DLL_RGWB void *csmarrayc_dontuse_get(struct csmarrayc_t *array, unsigned long idx);

DLL_RGWB void csmarrayc_dontuse_delete_element(struct csmarrayc_t *array, unsigned long idx, csmarrayc_FPtr_free_struct func_free);

DLL_RGWB void csmarrayc_dontuse_qsort(struct csmarrayc_t *array, csmarrayc_FPtr_compare func_compare);

DLL_RGWB void csmarrayc_dontuse_qsort_1_extra(
                        struct csmarrayc_t *array,
                        struct csmarrayc_extra_item_t *extra_item,
                        csmarrayc_FPtr_compare_1_extra func_compare_1_extra);

DLL_RGWB void csmarrayc_dontuse_invert(struct csmarrayc_t *array);

// Array of pointers to structs...

#define csmarrayc_new_st_array(capacidad_inicial, type) (csmArrayStruct(type) *)csmarrayc_dontuse_new_ptr_array(capacidad_inicial, sizeof(struct type *))
#define csmarrayc_new_const_st_array(capacidad_inicial, type) (const csmArrayStruct(type) *)csmarrayc_dontuse_new_ptr_array(capacidad_inicial, sizeof(struct type *))

#define csmarrayc_copy_st_array(array, type, func_copy_element)\
(/*lint -save -e505*/\
    (void)((const csmArrayStruct(type) *)array == array),\
    CSMARRAYC_CHECK_FUNC_COPY_STRUCT(func_copy_element, type),\
    (csmArrayStruct(type) *)csmarrayc_dontuse_copy_ptr_array((struct csmarrayc_t *)array, (csmarrayc_FPtr_copy_struct)func_copy_element)\
)/*lint -restore*/

#define csmarrayc_free_st(array, type, func_free)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(type) **)array == array),\
    CSMARRAYC_CHECK_FUNC_FREE_STRUCT(func_free, type),\
    csmarrayc_dontuse_free((struct csmarrayc_t **)array, (csmarrayc_FPtr_free_struct)func_free)\
)/*lint -restore*/

#define csmarrayc_free_const_st(array, type)\
(/*lint -save -e505*/\
    (void)((const csmArrayStruct(type) **)array == array),\
    csmarrayc_dontuse_free((struct csmarrayc_t **)array, NULL)\
)/*lint -restore*/

#define csmarrayc_count_st(array, type)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(type) *)array == array),\
    csmarrayc_dontuse_count((const struct csmarrayc_t *)array)\
)

#define csmarrayc_insert_element_st(array, pos, element, type)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(type) *)array == array),\
    (void)((struct type *)element == element),\
    csmarrayc_dontuse_insert_element((struct csmarrayc_t *)array, pos, (void *)(&(element)))\
)/*lint -restore*/

#define csmarrayc_append_element_st(array, element, type)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(type) *)array == array),\
    (void)((struct type *)element == element),\
    csmarrayc_dontuse_append_element((struct csmarrayc_t *)array, (void *)(&(element)))\
)/*lint -restore*/

#define csmarrayc_append_element_const_st(array, element, type)\
(/*lint -save -e505*/\
    (void)((const csmArrayStruct(type) *)array == array),\
    (void)((const struct type *)element == element),\
    csmarrayc_dontuse_append_element((struct csmarrayc_t *)array, (void *)(&(element)))\
)/*lint -restore*/

#define csmarrayc_concat_st(array, array_origin, func_copy_element, type)\
    (/*lint -save -e505*/\
     (void)((csmArrayStruct(type) *)array == array),\
     (void)((const csmArrayStruct(type) *)array_origin == array_origin),\
     CSMARRAYC_CHECK_FUNC_COPY_STRUCT(func_copy_element, type),\
     csmarrayc_dontuse_concat((struct csmarrayc_t *)array, (const struct csmarrayc_t *)array_origin, (csmarrayc_FPtr_copy_struct)func_copy_element)\
     )/*lint -restore*/

#define csmarrayc_concat_const_st(array, array_origin, type)\
    (/*lint -save -e505*/\
     (void)((const csmArrayStruct(type) *)array == array),\
     (void)((const csmArrayStruct(type) *)array_origin == array_origin),\
     csmarrayc_dontuse_concat((struct csmarrayc_t *)array, (const struct csmarrayc_t *)array_origin, NULL)\
     )/*lint -restore*/

#define csmarrayc_contains_element_st(array, array_type, search_data, search_data_type, func_match_condition, idx_opt)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(array_type) *)array == array),\
    (void)((search_data_type *)search_data == search_data),\
    CSMARRAYC_CHECK_FUNC_MATCH_CONDITION(func_match_condition, array_type, search_data_type),\
    csmarrayc_dontuse_contains_element((struct csmarrayc_t *)array, (const void *)search_data, (csmarrayc_FPtr_match_condition)func_match_condition, idx_opt)\
)/*lint -restore*/

#define csmarrayc_get_st(array, idx, type)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(type) *)array == array),\
    (struct type *)csmarrayc_dontuse_get((struct csmarrayc_t *)array, idx)\
)/*lint -restore*/

#define csmarrayc_get_const_st(array, idx, type)\
(/*lint -save -e505*/\
    (void)((const csmArrayStruct(type) *)array == array),\
    (const struct type *)csmarrayc_dontuse_get((struct csmarrayc_t *)array, idx)\
)/*lint -restore*/

#define csmarrayc_set_st(array, idx, element, type)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(type) *)array == array),\
    (void)((struct type *)element == element),\
    csmarrayc_dontuse_set_element((struct csmarrayc_t *)array, idx, (void *)(element))\
)/*lint -restore*/

#define csmarrayc_set_const_st(array, idx, element, type)\
(/*lint -save -e505*/\
    (void)((const csmArrayStruct(type) *)array == array),\
    (void)((struct type *)element == element),\
    csmarrayc_dontuse_set_element((struct csmarrayc_t *)array, idx, (void *)(element))\
)/*lint -restore*/

#define csmarrayc_delete_element_st(array, idx, type, func_free)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(type) *)array == array),\
    CSMARRAYC_CHECK_FUNC_FREE_STRUCT(func_free, type),\
    csmarrayc_dontuse_delete_element((struct csmarrayc_t *)array, idx, (csmarrayc_FPtr_free_struct)func_free)\
)/*lint -restore*/

#define csmarrayc_qsort_st(array, type, func_compare)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(type) *)array == array),\
    CSMARRAYC_CHECK_FUNC_COMPARE_ST(func_compare, type),\
    csmarrayc_dontuse_qsort((struct csmarrayc_t *)array, (csmarrayc_FPtr_compare)func_compare)\
)/*lint -restore*/

#define csmarrayc_qsort_st_1_extra(array, type, extra_item, extra_type, func_compare)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(type) *)array == array),\
    (void)((extra_type *)extra_item == extra_item),\
    CSMARRAYC_CHECK_FUNC_COMPARE_ST_1_EXTRA(func_compare, type, extra_type),\
    csmarrayc_dontuse_qsort_1_extra((struct csmarrayc_t *)array, (struct csmarrayc_extra_item_t *)extra_item, (csmarrayc_FPtr_compare_1_extra)func_compare)\
)/*lint -restore*/

#define csmarrayc_invert(array, type)\
(/*lint -save -e505*/\
    (void)((csmArrayStruct(type) *)array == array),\
    csmarrayc_dontuse_invert((struct csmarrayc_t *)array)\
)/*lint -restore*/


#ifdef __cplusplus
}
#endif
