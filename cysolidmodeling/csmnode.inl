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

void csmnode_destruye(struct csmnode_t **node);


unsigned long csmnode_id(const struct csmnode_t *node);

struct csmnode_derivada_t *csmnode_nousar_downcast(struct csmnode_t *node, const char *tipo_clase_derivada);
#define csmnode_downcast(node, tipo_clase_derivada) (struct tipo_clase_derivada *)csmnode_nousar_downcast(node, #tipo_clase_derivada)

struct csmnode_t *csmnode_next(struct csmnode_t *node);
void csmnode_set_ptr_next(struct csmnode_t *node, struct csmnode_t *next_node);

struct csmnode_t *csmnode_prev(struct csmnode_t *node);
void csmnode_set_ptr_prev(struct csmnode_t *node, struct csmnode_t *prev_node);

void csmnode_nousar_remove_from_own_list(struct csmnode_t *node2, const char *tipo_clase_derivada);
#define csmnode_remove_from_own_list(node2, tipo_clase_derivada)\
(\
    ((struct tipo_clase_derivada *)node2 == node2),\
    csmnode_nousar_remove_from_own_list(CSMNODE(node2), #tipo_clase_derivada)\
)

void csmnode_nousar_insert_node2_before_node1(struct csmnode_t *node1, struct csmnode_t *node2, const char *tipo_clase_derivada);
#define csmnode_insert_node2_before_node1(node1, node2, tipo_clase_derivada)\
(\
    ((struct tipo_clase_derivada *)node1 == node1),\
    ((struct tipo_clase_derivada *)node2 == node2),\
    csmnode_nousar_insert_node2_before_node1(CSMNODE(node1), CSMNODE(node2), #tipo_clase_derivada)\
)

void csmnode_nousar_free_node_list(struct csmnode_derivada_t **head_node_derived, const char *tipo_clase_derivada);
#define csmnode_free_node_list(head_node_derived, tipo_clase_derivada)\
(\
    ((struct tipo_clase_derivada **)head_node_derived == head_node_derived),\
    csmnode_nousar_free_node_list((struct csmnode_derivada_t **)head_node_derived, #tipo_clase_derivada)\
)

void csmnode_nousar_free_node_in_list(struct csmnode_derivada_t **head_node_derived, const char *tipo_clase_derivada);
#define csmnode_free_node_in_list(head_node_derived, tipo_clase_derivada)\
(\
    ((struct tipo_clase_derivada **)head_node_derived == head_node_derived),\
    csmnode_nousar_free_node_in_list((struct csmnode_derivada_t **)head_node_derived, #tipo_clase_derivada)\
)
