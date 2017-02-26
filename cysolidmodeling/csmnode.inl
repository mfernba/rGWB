//csmnode_
//  csmnode.h
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"
#include "csmnode.hxx"

struct csmnode_t csmnode_nousar_crea_node(
                        unsigned long id,
                        struct csmnode_derivada_t *clase_derivada,
                        const char *tipo_clase_derivada,
                        csmnode_FPtr_destruye func_destruye_clase_derivada);
#define csmnode_crea_node(id, clase_derivada, func_destruye_clase_derivada, tipo_clase_derivada)\
(\
    ((struct tipo_clase_derivada *)clase_derivada == clase_derivada),\
    CSMNODE_COMPRUEBA_FUNC_DESTRUYE(func_destruye_clase_derivada, tipo_clase_derivada),\
    csmnode_nousar_crea_node(\
                id,\
                (struct csmnode_derivada_t *)clase_derivada,\
                #tipo_clase_derivada,\
                (csmnode_FPtr_destruye)func_destruye_clase_derivada)\
)

void csmnode_retain(struct csmnode_t *node);

struct csmnode_derivada_t *csmnode_nousar_retain_ex(struct csmnode_t *node, const char *tipo_clase_derivada);
#define csmnode_retain_ex(node, tipo_clase_derivada) (struct tipo_clase_derivada *)csmnode_nousar_retain_ex(CSMNODE(node), #tipo_clase_derivada)

void csmnode_release(struct csmnode_t *node);

void csmnode_nousar_release_ex(struct csmnode_t **node, const char *tipo_clase_derivada);
#define csmnode_release_ex(node, tipo_clase_derivada)\
(\
    ((struct tipo_clase_derivada **)node == node),\
    csmnode_nousar_release_ex((struct csmnode_t **)node, #tipo_clase_derivada)\
)


struct csmnode_derivada_t *csmnode_nousar_downcast(struct csmnode_t *node, const char *tipo_clase_derivada);
#define csmnode_downcast(node, tipo_clase_derivada) (struct tipo_clase_derivada *)csmnode_nousar_downcast(node, #tipo_clase_derivada)

struct csmnode_t *csmnode_next(struct csmnode_t *node);
void csmnode_set_ptr_next(struct csmnode_t *node, struct csmnode_t *next_node);

struct csmnode_t *csmnode_prev(struct csmnode_t *node);
void csmnode_set_ptr_prev(struct csmnode_t *node, struct csmnode_t *prev_node);

void csmnode_nousar_insert_node2_before_node1(struct csmnode_t *node1, struct csmnode_t *node2, const char *tipo_clase_derivada);
#define csmnode_insert_node2_before_node1(node1, node2, tipo_clase_derivada)\
(\
    ((struct tipo_clase_derivada *)node1 == node1),\
    ((struct tipo_clase_derivada *)node2 == node2),\
    csmnode_nousar_insert_node2_before_node1(CSMNODE(node1), CSMNODE(node2), #tipo_clase_derivada)\
)
