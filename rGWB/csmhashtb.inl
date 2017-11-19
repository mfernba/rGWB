// Hash Table for pairs (id, pointers)...

#include "csmhashtb.hxx"
#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmhashtb_t *, csmhashtb_nousar_create_empty, (void));
#define csmhashtb_create_empty(type) (struct csmhashtb(type) *)csmhashtb_nousar_create_empty()

void csmhashtb_nousar_free(struct csmhashtb_t **tabla, csmhashtb_FPtr_free_item_ptr func_free_item_ptr);
#define csmhashtb_free(tabla, type, func_free_item_ptr)\
(\
    ((struct csmhashtb(type) **)tabla == tabla),\
    CSMHASHTB_CHECK_FUNC_FREE_ITEM_PTR(func_free_item_ptr, type),\
    csmhashtb_nousar_free((struct csmhashtb_t **)tabla, (csmhashtb_FPtr_free_item_ptr)func_free_item_ptr)\
)

// Items...

unsigned long csmhashtb_nousar_count(const struct csmhashtb_t *tabla);
#define csmhashtb_count(tabla, type)\
(\
    (void)((struct csmhashtb(type) *)tabla == tabla),\
    csmhashtb_nousar_count((struct csmhashtb_t *)tabla)\
)

void csmhashtb_nousar_add_item(struct csmhashtb_t *tabla, unsigned long id, void *ptr);
#define csmhashtb_add_item(tabla, id, ptr, type)\
(\
    ((struct csmhashtb(type) *)tabla == tabla),\
    ((struct type *)ptr == ptr),\
    csmhashtb_nousar_add_item((struct csmhashtb_t *)tabla, id, (void *)ptr)\
)

void csmhashtb_nousar_remove_item(struct csmhashtb_t *tabla, unsigned long id);
#define csmhashtb_remove_item(tabla, id, type)\
(\
    ((struct csmhashtb(type) *)tabla == tabla),\
    csmhashtb_nousar_remove_item((struct csmhashtb_t *)tabla, id)\
)

void csmhashtb_nousar_clear(struct csmhashtb_t *tabla, csmhashtb_FPtr_free_item_ptr func_free_item_ptr);
#define csmhashtb_clear(tabla, type, func_free_item_ptr)\
(\
    ((struct csmhashtb(type) *)tabla == tabla),\
    CSMHASHTB_CHECK_FUNC_FREE_ITEM_PTR(func_free_item_ptr, type),\
    csmhashtb_nousar_clear((struct csmhashtb_t *)tabla, (csmhashtb_FPtr_free_item_ptr)func_free_item_ptr)\
)

// Search...

void *csmhashtb_nousar_ptr_for_id(struct csmhashtb_t *tabla, unsigned long id);
#define csmhashtb_ptr_for_id(tabla, id, type)\
(\
    ((struct csmhashtb(type) *)tabla == tabla),\
    (struct type *)csmhashtb_nousar_ptr_for_id((struct csmhashtb_t *)tabla, id)\
)

CSMBOOL csmhashtb_nousar_contains_id(struct csmhashtb_t *tabla, unsigned long id, void **ptr_opc);
#define csmhashtb_contains_id(tabla, type, id, ptr_opc)\
(\
    (void)((struct csmhashtb(type) *)tabla == tabla),\
    (void)((struct type **)ptr_opc == ptr_opc),\
    csmhashtb_nousar_contains_id((struct csmhashtb_t *)tabla, id, (void **)ptr_opc)\
)

// Iterators......

CONSTRUCTOR(struct csmhashtb_iterator_t *, csmhashtb_nousar_create_iterator, (struct csmhashtb_t *tabla));
#define csmhashtb_create_iterator(tabla, type)\
(\
    (void)((struct csmhashtb(type) *)tabla == tabla),\
    (struct csmhashtb_iterator(type) *)csmhashtb_nousar_create_iterator((struct csmhashtb_t *)tabla)\
)

void csmhashtb_nousar_free_iterator(struct csmhashtb_iterator_t **iterator);
#define csmhashtb_free_iterator(iterator, type)\
(\
    (void)((struct csmhashtb_iterator(type) **)iterator == iterator),\
    csmhashtb_nousar_free_iterator((struct csmhashtb_iterator_t **)iterator)\
)

CSMBOOL csmhashtb_nousar_has_next(const struct csmhashtb_iterator_t *iterator);
#define csmhashtb_has_next(iterator, type)\
(\
    (void)((const struct csmhashtb_iterator(type) *)iterator == iterator),\
    csmhashtb_nousar_has_next((const struct csmhashtb_iterator_t *)iterator)\
)

void csmhashtb_nousar_next_pair(struct csmhashtb_iterator_t *iterator, unsigned long *id_opc, void **ptr_opc);
#define csmhashtb_next_pair(iterator, id_opc, ptr_opc, type)\
(\
    (void)((struct csmhashtb_iterator(type) *)iterator == iterator),\
    (void)((struct type **)ptr_opc == ptr_opc),\
    csmhashtb_nousar_next_pair((struct csmhashtb_iterator_t *)iterator, id_opc, (void **)ptr_opc)\
)





