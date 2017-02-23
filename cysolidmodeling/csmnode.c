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

void csmnode_release(struct csmnode_t *node)
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
    }
}

// ------------------------------------------------------------------------------------------

void csmnode_nousar_release_ex(struct csmnode_t *node, const char *tipo_clase_derivada)
{
    if (node != NULL)
    {
        assert(cad_cadenas_iguales(node->tipo_clase_derivada, tipo_clase_derivada) == CIERTO);
        csmnode_release(node);
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
    assert_no_null((ptr));
    
    if (*ptr != NULL)
        csmnode_release(*ptr);

    *ptr = next_or_prev_vertex;
    
    if (next_or_prev_vertex != NULL)
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
