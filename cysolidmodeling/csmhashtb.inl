// Hash Table for pairs (id, pointers)...

#include "csmhashtb.hxx"
#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmhashtb_t *, csmhashtb_nousar_create_empty, (void));
#define csmhashtb_create_empty(type) (struct csmhashtb(type) *)csmhashtb_nousar_create_empty()

void csmhashtb_nousar_destruye(struct csmhashtb_t **tabla);
#define csmhashtb_destruye(tabla, type)\
(\
    ((struct csmhashtb(type) **)tabla == tabla),\
    csmhashtb_nousar_destruye((struct csmhashtb_t **)tabla)\
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

void *csmhashtb_nousar_ptr_for_id(struct csmhashtb_t *tabla, unsigned long id);
#define csmhashtb_ptr_for_id(tabla, id, type)\
(\
    ((struct csmhashtb(type) *)tabla == tabla),\
    (struct type *)csmhashtb_nousar_ptr_for_id((struct csmhashtb_t *)tabla, id)\
)

CYBOOL csmhashtb_nousar_contains_id(struct csmhashtb_t *tabla, unsigned long id, void **ptr_opc);
#define csmhashtb_contains_id(tabla, type, id, ptr_opc)\
(\
    ((struct csmhashtb(type) *)tabla == tabla),\
    ((struct type **)ptr_opc == ptr_opc),\
    csmhashtb_nousar_contains_id((struct csmhashtb_t *)tabla, id, (void **)ptr_opc)\
)
