//csmnode_
//  csmnode.h
//  rGWB
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"
#include "csmnode.hxx"

void csmnode_dontuse_init(
                        struct csmnode_t *node,
                        unsigned long id,
                        enum csmnode_object_t real_type,
                        csmnode_FPtr_free func_free);
#define csmnode_init(node, id, func_free, derived_class_type)\
(/*lint -save -e505*/\
    CSMNODE_CHECK_FUNC_FREE(func_free, derived_class_type),\
    csmnode_dontuse_init(\
                node,\
                id,\
                CSMNODE_OBJECT_##derived_class_type,\
                (csmnode_FPtr_free)func_free)\
)/*lint -restore*/


void csmnode_dealloc(struct csmnode_t *node);


unsigned long csmnode_id(const struct csmnode_t *node);

struct csmnode_t *csmnode_dontuse_downcast(struct csmnode_t *node, enum csmnode_object_t real_type);
#define csmnode_downcast(node, derived_class_type) (struct derived_class_type *)csmnode_dontuse_downcast(node, CSMNODE_OBJECT_##derived_class_type)

struct csmnode_t *csmnode_next(struct csmnode_t *node);
void csmnode_set_ptr_next(struct csmnode_t *node, struct csmnode_t *next_node);

struct csmnode_t *csmnode_prev(struct csmnode_t *node);
void csmnode_set_ptr_prev(struct csmnode_t *node, struct csmnode_t *prev_node);

void csmnode_dontuse_remove_from_own_list(struct csmnode_t *node2, enum csmnode_object_t real_type);
#define csmnode_remove_from_own_list(node2, derived_class_type)\
(/*lint -save -e505*/\
    ((struct derived_class_type *)node2 == node2),\
    csmnode_dontuse_remove_from_own_list(CSMNODE(node2), CSMNODE_OBJECT_##derived_class_type)\
)/*lint -restore*/

void csmnode_dontuse_insert_node2_before_node1(struct csmnode_t *node1, struct csmnode_t *node2, enum csmnode_object_t real_type);
#define csmnode_insert_node2_before_node1(node1, node2, derived_class_type)\
(/*lint -save -e505*/\
    ((struct derived_class_type *)node1 == node1),\
    ((struct derived_class_type *)node2 == node2),\
    csmnode_dontuse_insert_node2_before_node1(CSMNODE(node1), CSMNODE(node2), CSMNODE_OBJECT_##derived_class_type)\
)/*lint -restore*/

void csmnode_dontuse_insert_node2_after_node1(struct csmnode_t *node1, struct csmnode_t *node2, enum csmnode_object_t real_type);
#define csmnode_insert_node2_after_node1(node1, node2, derived_class_type)\
(/*lint -save -e505*/\
    ((struct derived_class_type *)node1 == node1),\
    ((struct derived_class_type *)node2 == node2),\
    csmnode_dontuse_insert_node2_after_node1(CSMNODE(node1), CSMNODE(node2), CSMNODE_OBJECT_##derived_class_type)\
)/*lint -restore*/

void csmnode_dontuse_free_node_list(struct csmnode_t **head_node_derived, enum csmnode_object_t real_type);
#define csmnode_free_node_list(head_node_derived, derived_class_type)\
(/*lint -save -e505*/\
    ((struct derived_class_type **)head_node_derived == head_node_derived),\
    csmnode_dontuse_free_node_list((struct csmnode_t **)head_node_derived, CSMNODE_OBJECT_##derived_class_type)\
)/*lint -restore*/

void csmnode_dontuse_free_node_in_list(struct csmnode_t **head_node_derived, enum csmnode_object_t real_type);
#define csmnode_free_node_in_list(head_node_derived, derived_class_type)\
(/*lint -save -e505*/\
    ((struct derived_class_type **)head_node_derived == head_node_derived),\
    csmnode_dontuse_free_node_in_list((struct csmnode_t **)head_node_derived, CSMNODE_OBJECT_##derived_class_type)\
)/*lint -restore*/
