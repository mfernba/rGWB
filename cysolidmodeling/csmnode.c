//
//  csmnode.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmnode.inl"

#include "cyassert.h"
#include "cypespy.h"
#include "cypestr.h"

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
    node.tipo_clase_derivada = cad_copia_cadena(tipo_clase_derivada);
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
    
    cypestr_destruye(&node_loc->tipo_clase_derivada);
    node_loc->func_destruye_clase_derivada(&node_loc->clase_derivada);
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
        assert(cad_cadenas_iguales(node->tipo_clase_derivada, tipo_clase_derivada) == CIERTO);
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
    assert(cad_cadenas_iguales(node2->tipo_clase_derivada, tipo_clase_derivada) == CIERTO);
    
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
    assert(cad_cadenas_iguales(node1->tipo_clase_derivada, tipo_clase_derivada) == CIERTO);
    assert(cad_cadenas_iguales(node2->tipo_clase_derivada, tipo_clase_derivada) == CIERTO);

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

void csmnode_nousar_free_node_list(struct csmnode_derivada_t **head_node_derived, const char *tipo_clase_derivada)
{
    struct csmnode_derivada_t *head_node_derived_loc;
    struct csmnode_t *head_node;
    
    head_node_derived_loc = ASIGNA_PUNTERO_PP_NO_NULL(head_node_derived, struct csmnode_derivada_t);
    head_node = (struct csmnode_t *)head_node_derived_loc;
    
    do
    {
        struct csmnode_t *next_node;
        
        if (head_node->next != head_node)
            next_node = head_node->next;
        else
            next_node = NULL;
        
        csmnode_destruye(&head_node);
        
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





