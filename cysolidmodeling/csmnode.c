//
//  csmnode.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmnode.inl"

#include "cyassert.h"
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
    node.num_referencias = 1;
    
    node.clase_derivada = clase_derivada;
    node.tipo_clase_derivada = cad_copia_cadena(tipo_clase_derivada);
    node.func_destruye_clase_derivada = func_destruye_clase_derivada;
    
    node.prev = NULL;
    node.next = NULL;
    
    return node;
}

// ------------------------------------------------------------------------------------------

void csmnode_retain(struct csmnode_t *node)
{
    if (node != NULL)
        node->num_referencias++;
}

// ------------------------------------------------------------------------------------------

struct csmnode_derivada_t *csmnode_nousar_retain_ex(struct csmnode_t *node, const char *tipo_clase_derivada)
{
    if (node != NULL)
    {
        assert(cad_cadenas_iguales(node->tipo_clase_derivada, tipo_clase_derivada) == CIERTO);
        return node->clase_derivada;
    }
    else
    {
        return NULL;
    }
}

// ------------------------------------------------------------------------------------------

static void i_release(struct csmnode_t *node, CYBOOL check_must_be_destroyed)
{
    if (node != NULL)
    {
        assert_no_null(node->func_destruye_clase_derivada);
        assert(node->num_referencias >= 2);
        
        node->num_referencias--;
        
        if (node->num_referencias == 0)
        {
            cypestr_destruye(&node->tipo_clase_derivada);
            node->func_destruye_clase_derivada(&node->clase_derivada);
        }
        else
        {
            assert(check_must_be_destroyed == FALSO);
        }
    }
    else
    {
        assert(check_must_be_destroyed == FALSO);
    }
}

// ------------------------------------------------------------------------------------------

void csmnode_release(struct csmnode_t *node)
{
    CYBOOL check_must_be_destroyed;
    
    check_must_be_destroyed = FALSO;
    i_release(node, check_must_be_destroyed);
}

// ------------------------------------------------------------------------------------------

void csmnode_nousar_release_ex(struct csmnode_t **node, const char *tipo_clase_derivada, CYBOOL check_must_be_destroyed)
{
    assert_no_null(node);
    
    if (*node != NULL)
    {
        assert(cad_cadenas_iguales((*node)->tipo_clase_derivada, tipo_clase_derivada) == CIERTO);
        
        i_release(*node, check_must_be_destroyed);
        *node = NULL;
    }
}

// ------------------------------------------------------------------------------------------

struct csmnode_derivada_t *csmnode_nousar_downcast(struct csmnode_t *node, const char *tipo_clase_derivada)
{
    assert_no_null(node);
    assert(cad_cadenas_iguales(node->tipo_clase_derivada, tipo_clase_derivada) == CIERTO);
    
    return node->clase_derivada;
}

// ------------------------------------------------------------------------------------------

static void i_set_ptr_next_or_prev(struct csmnode_t **ptr, struct csmnode_t *next_or_prev_vertex)
{
    assert_no_null(ptr);
    
    csmnode_release(*ptr);

    *ptr = next_or_prev_vertex;
    
    csmnode_retain(next_or_prev_vertex);
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
    assert_no_null(next_node);
    
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
    assert_no_null(prev_node);
    
    i_set_ptr_next_or_prev(&node->prev, prev_node);
}

// ------------------------------------------------------------------------------------------

void csmnode_nousar_insert_node2_before_node1(struct csmnode_t *node1, struct csmnode_t *node2, const char *tipo_clase_derivada)
{
    assert_no_null(node1);
    assert_no_null(node2);
    assert(cad_cadenas_iguales(node1->tipo_clase_derivada, tipo_clase_derivada) == CIERTO);
    assert(cad_cadenas_iguales(node2->tipo_clase_derivada, tipo_clase_derivada) == CIERTO);

    if (node1->prev != NULL)
    {
        assert(node1->prev->next == node1);

        i_set_ptr_next_or_prev(&node2->prev, node1->prev);
        i_set_ptr_next_or_prev(&node1->prev->next, node2);
    }

    i_set_ptr_next_or_prev(&node1->prev, node2);
    i_set_ptr_next_or_prev(&node2->next, node1);
}


