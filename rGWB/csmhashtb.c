/*lint -save -w0*/

//  csmhashtb.c
//  rGWB
//
//  Created by Manuel Fernández on 16/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmhashtb.inl"

#include "csmuthash.hxx"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

struct csmhashtb_item_t
{
    unsigned long id;
    struct csmhashtb_item_ptr_t *ptr;
    
    UT_hash_handle hh;
};

struct csmhashtb_t
{
    struct csmhashtb_item_t *items;
};

struct csmhashtb_iterator_t
{
    struct csmhashtb_item_t *next_item;
};

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmhashtb_item_t *, i_crea_item, (unsigned long id, void *ptr))
{
    struct csmhashtb_item_t *item;
    
    item = MALLOC(struct csmhashtb_item_t);
    
    item->id = id;
    item->ptr = ptr;
    
    return item;
}

// ------------------------------------------------------------------------------------------

static void i_destruye_item(struct csmhashtb_item_t **item, csmhashtb_FPtr_free_item_ptr func_free_item_ptr)
{
    assert_no_null(item);
    assert_no_null(*item);
    
    if (func_free_item_ptr != NULL)
        func_free_item_ptr(&(*item)->ptr);
    
    FREE_PP(item, struct csmhashtb_item_t);
}

// ------------------------------------------------------------------------------------------

struct csmhashtb_t *csmhashtb_nousar_create_empty(void)
{
    struct csmhashtb_t *tabla;
    
    tabla = MALLOC(struct csmhashtb_t);
    tabla->items = NULL;
    
    return tabla;
}

// ------------------------------------------------------------------------------------------

void csmhashtb_nousar_free(struct csmhashtb_t **tabla, csmhashtb_FPtr_free_item_ptr func_free_item_ptr)
{
    assert_no_null(tabla);
    assert_no_null(*tabla);
    
    if ((*tabla)->items != NULL)
    {
        struct csmhashtb_item_t *current_item, *tmp;

        HASH_ITER(hh, (*tabla)->items, current_item, tmp)
        {
            HASH_DEL((*tabla)->items, current_item);
            i_destruye_item(&current_item, func_free_item_ptr);
        }
    }
    
    FREE_PP(tabla, struct csmhashtb_t);
}

// ------------------------------------------------------------------------------------------

unsigned long csmhashtb_nousar_count(const struct csmhashtb_t *tabla)
{
    assert_no_null(tabla);
    return HASH_COUNT(tabla->items);
}

// ------------------------------------------------------------------------------------------

void csmhashtb_nousar_add_item(struct csmhashtb_t *tabla, unsigned long id, void *ptr)
{
    struct csmhashtb_item_t *item;
    
    assert_no_null(tabla);
    
    item = i_crea_item(id, ptr);
    HASH_ADD_INT(tabla->items, id, item);
}

// ------------------------------------------------------------------------------------------

void csmhashtb_nousar_remove_item(struct csmhashtb_t *tabla, unsigned long id)
{
    struct csmhashtb_item_t *item;
    
    assert_no_null(tabla);
    
    item = NULL;
    
    HASH_FIND_INT(tabla->items, &id, item);
    assert_no_null(item);
    
    HASH_DEL(tabla->items, item);
    FREE_PP(&item, struct csmhashtb_item_t);
}

// ------------------------------------------------------------------------------------------

void csmhashtb_nousar_clear(struct csmhashtb_t *tabla, csmhashtb_FPtr_free_item_ptr func_free_item_ptr)
{
    assert_no_null(tabla);
    
    if (tabla->items != NULL)
    {
        struct csmhashtb_item_t *current_item, *tmp;

        HASH_ITER(hh, tabla->items, current_item, tmp)
        {
            HASH_DEL(tabla->items, current_item);
            i_destruye_item(&current_item, func_free_item_ptr);
        }
    }
}

// ------------------------------------------------------------------------------------------

void *csmhashtb_nousar_ptr_for_id(struct csmhashtb_t *tabla, unsigned long id)
{
    struct csmhashtb_item_t *item;
    
    assert_no_null(tabla);
    
    HASH_FIND_INT(tabla->items, &id, item);
    assert_no_null(item);
    
    return item->ptr;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmhashtb_nousar_contains_id(struct csmhashtb_t *tabla, unsigned long id, void **ptr_opc)
{
    struct csmhashtb_item_t *item;
    
    assert_no_null(tabla);
    
    HASH_FIND_INT(tabla->items, &id, item);
    
    if (item != NULL)
    {
        ASSIGN_OPTIONAL_VALUE(ptr_opc, item->ptr);
        return CSMTRUE;
    }
    else
    {
        return CSMFALSE;
    }
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmhashtb_iterator_t *, i_create_iterator, (struct csmhashtb_item_t *next_item))
{
    struct csmhashtb_iterator_t *iterator;
    
    iterator = MALLOC(struct csmhashtb_iterator_t);
    
    iterator->next_item = next_item;
    
    return iterator;
}

// ------------------------------------------------------------------------------------------

struct csmhashtb_iterator_t *csmhashtb_nousar_create_iterator(struct csmhashtb_t *tabla)
{
    assert_no_null(tabla);
    return i_create_iterator(tabla->items);
}

// ------------------------------------------------------------------------------------------

void csmhashtb_nousar_free_iterator(struct csmhashtb_iterator_t **iterator)
{
    assert_no_null(iterator);
    assert_no_null(*iterator);
    
    FREE_PP(iterator, struct csmhashtb_iterator_t);
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmhashtb_nousar_has_next(const struct csmhashtb_iterator_t *iterator)
{
    assert_no_null(iterator);
    return IS_TRUE(iterator->next_item != NULL);
}

// ------------------------------------------------------------------------------------------

void csmhashtb_nousar_next_pair(struct csmhashtb_iterator_t *iterator, unsigned long *id_opc, void **ptr_opc)
{
    assert_no_null(iterator);
    assert_no_null(iterator->next_item);
    
    ASSIGN_OPTIONAL_VALUE(id_opc, iterator->next_item->id);
    ASSIGN_OPTIONAL_VALUE(ptr_opc, iterator->next_item->ptr);
    
    iterator->next_item = iterator->next_item->hh.next;
}

/*lint -restore*/
