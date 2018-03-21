//
//  csmnode.c
//  rGWB
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmnode.inl"

#include "csmstring.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

// ------------------------------------------------------------------------------------------

struct csmnode_t csmnode_nousar_crea_node(
                        unsigned long id,
                        struct csmnode_derivada_t *clase_derivada,
                        const char *tipo_clase_derivada,
                        csmnode_FPtr_destruye func_destruye_clase_derivada)
{
    struct csmnode_t node;
    
    node.id = id;
    
    node.clase_derivada = clase_derivada;
    node.tipo_clase_derivada = csmstring_duplicate(tipo_clase_derivada);
    node.func_destruye_clase_derivada = func_destruye_clase_derivada;
    
    node.prev = NULL;
    node.next = NULL;
    
    return node;
}
// ------------------------------------------------------------------------------------------

void csmnode_destruye(struct csmnode_t **node)
{
    struct csmnode_t *node_loc;

    assert_no_null(node);
    assert_no_null(*node);
    assert_no_null((*node)->func_destruye_clase_derivada);
    
    node_loc = ASIGNA_PUNTERO_PP_NO_NULL(node, struct csmnode_t);
    
    csmstring_free(&node_loc->tipo_clase_derivada);
    node_loc->func_destruye_clase_derivada(&node_loc->clase_derivada);
}

// ------------------------------------------------------------------------------------------

unsigned long csmnode_id(const struct csmnode_t *node)
{
    assert_no_null(node);
    return node->id;
}

// ------------------------------------------------------------------------------------------

struct csmnode_derivada_t *csmnode_nousar_downcast(struct csmnode_t *node, const char *tipo_clase_derivada)
{
    if (node == NULL)
    {
        return NULL;
    }
    else
    {
        assert(csmstring_equal_strings(node->tipo_clase_derivada, tipo_clase_derivada) == CSMTRUE);
        return node->clase_derivada;
    }
}

// ------------------------------------------------------------------------------------------

static void i_set_ptr_next_or_prev(struct csmnode_t **ptr, struct csmnode_t *next_or_prev_vertex)
{
    assert_no_null(ptr);
    *ptr = next_or_prev_vertex;
}

// ------------------------------------------------------------------------------------------

struct csmnode_t *csmnode_next(struct csmnode_t *node)
{
    assert_no_null(node);
    return node->next;
}

// ------------------------------------------------------------------------------------------

void csmnode_set_ptr_next(struct csmnode_t *node, struct csmnode_t *next_node)
{
    assert_no_null(node);
    i_set_ptr_next_or_prev(&node->next, next_node);
}

// ------------------------------------------------------------------------------------------

struct csmnode_t *csmnode_prev(struct csmnode_t *node)
{
    assert_no_null(node);
    return node->prev;
}

// ------------------------------------------------------------------------------------------

void csmnode_set_ptr_prev(struct csmnode_t *node, struct csmnode_t *prev_node)
{
    assert_no_null(node);
    i_set_ptr_next_or_prev(&node->prev, prev_node);
}

// ------------------------------------------------------------------------------------------

void csmnode_nousar_remove_from_own_list(struct csmnode_t *node2, const char *tipo_clase_derivada)
{
    assert_no_null(node2);
    assert(csmstring_equal_strings(node2->tipo_clase_derivada, tipo_clase_derivada) == CSMTRUE);
    
    if (node2->prev != NULL)
    {
        assert(node2->prev->next == node2);
        node2->prev->next = node2->next;
    }

    if (node2->next != NULL)
    {
        assert(node2->next->prev == node2);
        node2->next->prev = node2->prev;
    }
    
    node2->prev = NULL;
    node2->next = NULL;
}

// ------------------------------------------------------------------------------------------

void csmnode_nousar_insert_node2_before_node1(struct csmnode_t *node1, struct csmnode_t *node2, const char *tipo_clase_derivada)
{
    assert_no_null(node1);
    assert_no_null(node2);
    assert(csmstring_equal_strings(node1->tipo_clase_derivada, tipo_clase_derivada) == CSMTRUE);
    assert(csmstring_equal_strings(node2->tipo_clase_derivada, tipo_clase_derivada) == CSMTRUE);

    csmnode_nousar_remove_from_own_list(node2, tipo_clase_derivada);
    
    if (node1->prev != NULL)
    {
        assert(node1->prev->next == node1);

        i_set_ptr_next_or_prev(&node2->prev, node1->prev);
        i_set_ptr_next_or_prev(&node1->prev->next, node2);
    }

    i_set_ptr_next_or_prev(&node1->prev, node2);
    i_set_ptr_next_or_prev(&node2->next, node1);
}

// ------------------------------------------------------------------------------------------

void csmnode_nousar_insert_node2_after_node1(struct csmnode_t *node1, struct csmnode_t *node2, const char *tipo_clase_derivada)
{
    assert_no_null(node1);
    assert_no_null(node2);
    assert(csmstring_equal_strings(node1->tipo_clase_derivada, tipo_clase_derivada) == CSMTRUE);
    assert(csmstring_equal_strings(node2->tipo_clase_derivada, tipo_clase_derivada) == CSMTRUE);

    csmnode_nousar_remove_from_own_list(node2, tipo_clase_derivada);
    
    if (node1->next != NULL)
    {
        assert(node1->next->prev == node1);

        i_set_ptr_next_or_prev(&node2->next, node1->next);
        i_set_ptr_next_or_prev(&node1->next->prev, node2);
    }

    i_set_ptr_next_or_prev(&node1->next, node2);
    i_set_ptr_next_or_prev(&node2->prev, node1);
}

// ------------------------------------------------------------------------------------------

void csmnode_nousar_free_node_list(struct csmnode_derivada_t **head_node_derived, const char *tipo_clase_derivada)
{
    struct csmnode_derivada_t *head_node_derived_loc;
    struct csmnode_t *first_node, *head_node;
    //unsigned long no_of_deleted_nodes;
    
    head_node_derived_loc = ASIGNA_PUNTERO_PP_NO_NULL(head_node_derived, struct csmnode_derivada_t);
    head_node = (struct csmnode_t *)head_node_derived_loc;
    first_node = head_node;
    //no_of_deleted_nodes = 0;
    
    do
    {
        struct csmnode_t *next_node;
        
        // head_node->next != head_node  -> mvfs solid (one face, one loop, one hedge, no edge, code as hegde->next == hegde
        // head_node->next != first_node -> loop edge list
        
        if (head_node->next != head_node && head_node->next != first_node)
            next_node = head_node->next;
        else
            next_node = NULL;
        
        csmnode_destruye(&head_node);
        //no_of_deleted_nodes++;
        
        head_node = next_node;
        
    } while (head_node != NULL);
}

// ------------------------------------------------------------------------------------------

void csmnode_nousar_free_node_in_list(struct csmnode_derivada_t **head_node_derived, const char *tipo_clase_derivada)
{
    struct csmnode_derivada_t *head_node_derived_loc;
    struct csmnode_t *head_node;
    
    head_node_derived_loc = ASIGNA_PUNTERO_PP_NO_NULL(head_node_derived, struct csmnode_derivada_t);
    head_node = (struct csmnode_t *)head_node_derived_loc;
    
    csmnode_nousar_remove_from_own_list(head_node, tipo_clase_derivada);
    csmnode_destruye(&head_node);
}





