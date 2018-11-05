//
//  csmnode.c
//  rGWB
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmnode.inl"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

// ------------------------------------------------------------------------------------------

void csmnode_dontuse_init(
                        struct csmnode_t *node,
                        unsigned long id,
                        enum csmnode_object_t real_type,
                        csmnode_FPtr_free func_free)
{
    assert_no_null(node);
    
    node->id = id;
    
    node->real_type = real_type;
    node->func_free = func_free;
    
    node->prev = NULL;
    node->next = NULL;
}

// ------------------------------------------------------------------------------------------

void csmnode_dealloc(struct csmnode_t *node)
{
    assert_no_null(node);
}

// ------------------------------------------------------------------------------------------

unsigned long csmnode_id(const struct csmnode_t *node)
{
    assert_no_null(node);
    return node->id;
}

// ------------------------------------------------------------------------------------------

struct csmnode_t *csmnode_dontuse_downcast(struct csmnode_t *node, enum csmnode_object_t real_type)
{
    if (node == NULL)
    {
        return NULL;
    }
    else
    {
        assert(node->real_type == real_type);
        return node;
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

void csmnode_dontuse_remove_from_own_list(struct csmnode_t *node2, enum csmnode_object_t real_type)
{
    assert_no_null(node2);
    assert(node2->real_type == real_type);
    
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

void csmnode_dontuse_insert_node2_before_node1(struct csmnode_t *node1, struct csmnode_t *node2, enum csmnode_object_t real_type)
{
    assert_no_null(node1);
    assert_no_null(node2);
    assert(node1->real_type == real_type);
    assert(node2->real_type == real_type);

    csmnode_dontuse_remove_from_own_list(node2, real_type);
    
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

void csmnode_dontuse_insert_node2_after_node1(struct csmnode_t *node1, struct csmnode_t *node2, enum csmnode_object_t real_type)
{
    assert_no_null(node1);
    assert_no_null(node2);
    assert(node1->real_type == real_type);
    assert(node2->real_type == real_type);

    csmnode_dontuse_remove_from_own_list(node2, real_type);
    
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

void csmnode_dontuse_free_node_list(struct csmnode_t **head_node_derived, enum csmnode_object_t real_type)
{
    struct csmnode_t *head_node_derived_loc;
    struct csmnode_t *first_node, *head_node;
    //unsigned long no_of_deleted_nodes;
    
    head_node_derived_loc = ASSIGN_POINTER_PP_NOT_NULL(head_node_derived, struct csmnode_t);
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
        
        assert_no_null(head_node->func_free);
        head_node->func_free(&head_node);
        //no_of_deleted_nodes++;
        
        head_node = next_node;
        
    } while (head_node != NULL);
}

// ------------------------------------------------------------------------------------------

void csmnode_dontuse_free_node_in_list(struct csmnode_t **head_node_derived, enum csmnode_object_t real_type)
{
    struct csmnode_t *head_node_derived_loc;
    struct csmnode_t *head_node;
    
    head_node_derived_loc = ASSIGN_POINTER_PP_NOT_NULL(head_node_derived, struct csmnode_t);
    head_node = (struct csmnode_t *)head_node_derived_loc;
    
    csmnode_dontuse_remove_from_own_list(head_node, real_type);
    
    assert_no_null(head_node->func_free);
    head_node->func_free(&head_node);
}





