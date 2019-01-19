// Hash Table for pairs (id, pointers)...

#include "csmhashtb.hxx"
#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmhashtb2id_t *, csmhashtb2id_nousar_create_empty, (void));
#define csmhashtb2id_create_empty(type) (struct csmhashtb2id(type) *)csmhashtb2id_nousar_create_empty()

void csmhashtb2id_nousar_free(struct csmhashtb2id_t **tabla, csmhashtb_FPtr_free_item_ptr func_free_item_ptr);
#define csmhashtb2id_free(tabla, type, func_free_item_ptr)\
(/*lint -save -e505*/\
    ((struct csmhashtb2id(type) **)tabla == tabla),\
    CSMHASHTB_CHECK_FUNC_FREE_ITEM_PTR(func_free_item_ptr, type),\
    csmhashtb2id_nousar_free((struct csmhashtb2id_t **)tabla, (csmhashtb_FPtr_free_item_ptr)func_free_item_ptr)\
)/*lint -restore*/

// Items...

unsigned long csmhashtb2id_nousar_count(const struct csmhashtb2id_t *tabla);
#define csmhashtb2id_count(tabla, type)\
(/*lint -save -e505*/\
    (void)((struct csmhashtb2id(type) *)tabla == tabla),\
    csmhashtb2id_nousar_count((struct csmhashtb2id_t *)tabla)\
)/*lint -restore*/

void csmhashtb2id_nousar_add_item(struct csmhashtb2id_t *tabla, unsigned long id1, unsigned long id2, void *ptr);
#define csmhashtb2id_add_item(tabla, id1, id2, ptr, type)\
(/*lint -save -e505*/\
    ((struct csmhashtb2id(type) *)tabla == tabla),\
    ((struct type *)ptr == ptr),\
    csmhashtb2id_nousar_add_item((struct csmhashtb2id_t *)tabla, id1, id2, (void *)ptr)\
)/*lint -restore*/

void csmhashtb2id_nousar_remove_item(struct csmhashtb2id_t *tabla, unsigned long id1, unsigned long id2);
#define csmhashtb2id_remove_item(tabla, id1, id2, type)\
(/*lint -save -e505*/\
    ((struct csmhashtb2id(type) *)tabla == tabla),\
    csmhashtb2id_nousar_remove_item((struct csmhashtb2id_t *)tabla, id1, id2)\
)/*lint -restore*/

void csmhashtb2id_nousar_clear(struct csmhashtb2id_t *tabla, csmhashtb_FPtr_free_item_ptr func_free_item_ptr);
#define csmhashtb2id_clear(tabla, type, func_free_item_ptr)\
(/*lint -save -e505*/\
    ((struct csmhashtb2id(type) *)tabla == tabla),\
    csmhashtb2id_CHECK_FUNC_FREE_ITEM_PTR(func_free_item_ptr, type),\
    csmhashtb2id_nousar_clear((struct csmhashtb2id_t *)tabla, (csmhashtb_FPtr_free_item_ptr)func_free_item_ptr)\
)/*lint -restore*/

// Search...

void *csmhashtb2id_nousar_ptr_for_id(struct csmhashtb2id_t *tabla, unsigned long id1, unsigned long id2);
#define csmhashtb2id_ptr_for_id(tabla, id, type)\
(/*lint -save -e505*/\
    ((struct csmhashtb2id(type) *)tabla == tabla),\
    (struct type *)csmhashtb2id_nousar_ptr_for_id((struct csmhashtb2id_t *)tabla, id)\
)/*lint -restore*/

CSMBOOL csmhashtb2id_nousar_contains_id(struct csmhashtb2id_t *tabla, unsigned long id1, unsigned long id2, void **ptr_opc);
#define csmhashtb2id_contains_id(tabla, type, id1, id2, ptr_opc)\
(/*lint -save -e505*/\
    (void)((struct csmhashtb2id(type) *)tabla == tabla),\
    (void)((struct type **)ptr_opc == ptr_opc),\
    csmhashtb2id_nousar_contains_id((struct csmhashtb2id_t *)tabla, id1, id2, (void **)ptr_opc)\
)/*lint -restore*/

// Iterators......

CONSTRUCTOR(struct csmhashtb2id_iterator_t *, csmhashtb2id_nousar_create_iterator, (struct csmhashtb2id_t *tabla));
#define csmhashtb2id_create_iterator(tabla, type)\
(/*lint -save -e505*/\
    (void)((struct csmhashtb2id(type) *)tabla == tabla),\
    (struct csmhashtb2id_iterator(type) *)csmhashtb2id_nousar_create_iterator((struct csmhashtb2id_t *)tabla)\
)/*lint -restore*/

void csmhashtb2id_nousar_free_iterator(struct csmhashtb2id_iterator_t **iterator);
#define csmhashtb2id_free_iterator(iterator, type)\
(/*lint -save -e505*/\
    (void)((struct csmhashtb2id_iterator(type) **)iterator == iterator),\
    csmhashtb2id_nousar_free_iterator((struct csmhashtb2id_iterator_t **)iterator)\
)/*lint -restore*/

CSMBOOL csmhashtb2id_nousar_has_next(const struct csmhashtb2id_iterator_t *iterator);
#define csmhashtb2id_has_next(iterator, type)\
(/*lint -save -e505*/\
    (void)((const struct csmhashtb2id_iterator(type) *)iterator == iterator),\
    csmhashtb2id_nousar_has_next((const struct csmhashtb2id_iterator_t *)iterator)\
)/*lint -restore*/

void csmhashtb2id_nousar_next_pair(struct csmhashtb2id_iterator_t *iterator, unsigned long *id1_opc, unsigned long *id2_opc, void **ptr_opc);
#define csmhashtb2id_next_pair(iterator, id1_opc, id2_opc, ptr_opc, type)\
(/*lint -save -e505*/\
    (void)((struct csmhashtb2id_iterator(type) *)iterator == iterator),\
    (void)((struct type **)ptr_opc == ptr_opc),\
    csmhashtb2id_nousar_next_pair((struct csmhashtb2id_iterator_t *)iterator, id1_opc, id2_opc, (void **)ptr_opc)\
)/*lint -restore*/

CONSTRUCTOR(csmArrayStruct(csmhashtb_item_ptr_t) *, csmhashtb2id_nousar_convert_to_array, (struct csmhashtb2id_t *tabla));
#define csmhashtb2id_convert_to_array(tabla, type)\
(/*lint -save -e505*/\
    (void)((struct csmhashtb2id(type) *)tabla == tabla),\
    (csmArrayStruct(type) *)csmhashtb2id_nousar_convert_to_array((struct csmhashtb2id_t *)tabla)\
)/*lint -restore*/
