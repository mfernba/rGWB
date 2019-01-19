/*lint -save -w0*/

//  csmhashtb2id.c
//  rGWB
//
//  Created by Manuel Fernández on 16/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmhashtb2id.inl"

#include "csmarrayc.h"
#include "csmuthash.hxx"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#include "csmmem.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

struct i_key_t
{
    unsigned long id1;
    unsigned long id2;
};

struct csmhashtb2id_item_t
{
    struct i_key_t key;
    struct csmhashtb_item_ptr_t *ptr;
    
    UT_hash_handle hh;
};

struct csmhashtb2id_t
{
    struct csmhashtb2id_item_t *items;
};

struct csmhashtb2id_iterator_t
{
    struct csmhashtb2id_item_t *next_item;
};

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmhashtb2id_item_t *, i_crea_item, (unsigned long id1, unsigned long id2, void *ptr))
{
    struct csmhashtb2id_item_t *item;
    
    item = MALLOC(struct csmhashtb2id_item_t);
    
    item->key.id1 = id1;
    item->key.id2 = id2;
    
    item->ptr = ptr;
    
    return item;
}

// ------------------------------------------------------------------------------------------

static void i_destruye_item(struct csmhashtb2id_item_t **item, csmhashtb_FPtr_free_item_ptr func_free_item_ptr)
{
    assert_no_null(item);
    assert_no_null(*item);
    
    if (func_free_item_ptr != NULL)
        func_free_item_ptr(&(*item)->ptr);
    
    FREE_PP(item, struct csmhashtb2id_item_t);
}

// ------------------------------------------------------------------------------------------

struct csmhashtb2id_t *csmhashtb2id_nousar_create_empty(void)
{
    struct csmhashtb2id_t *tabla;
    
    tabla = MALLOC(struct csmhashtb2id_t);
    tabla->items = NULL;
    
    return tabla;
}

// ------------------------------------------------------------------------------------------

void csmhashtb2id_nousar_free(struct csmhashtb2id_t **tabla, csmhashtb_FPtr_free_item_ptr func_free_item_ptr)
{
    assert_no_null(tabla);
    assert_no_null(*tabla);
    
    if ((*tabla)->items != NULL)
    {
        struct csmhashtb2id_item_t *current_item, *tmp;

        HASH_ITER(hh, (*tabla)->items, current_item, tmp)
        {
            HASH_DEL((*tabla)->items, current_item);
            i_destruye_item(&current_item, func_free_item_ptr);
        }
    }
    
    FREE_PP(tabla, struct csmhashtb2id_t);
}

// ------------------------------------------------------------------------------------------

unsigned long csmhashtb2id_nousar_count(const struct csmhashtb2id_t *tabla)
{
    assert_no_null(tabla);
    return HASH_COUNT(tabla->items);
}

// ------------------------------------------------------------------------------------------

void csmhashtb2id_nousar_add_item(struct csmhashtb2id_t *tabla, unsigned long id1, unsigned long id2, void *ptr)
{
    struct csmhashtb2id_item_t *item;
    
    assert_no_null(tabla);
    
    item = i_crea_item(id1, id2, ptr);
    HASH_ADD(hh, tabla->items, key, sizeof(struct i_key_t), item);
}

// ------------------------------------------------------------------------------------------

void csmhashtb2id_nousar_remove_item(struct csmhashtb2id_t *tabla, unsigned long id1, unsigned long id2)
{
    struct csmhashtb2id_item_t *item;
    struct i_key_t key;
    
    assert_no_null(tabla);
    
    key.id1 = id1;
    key.id2 = id2;
    item = NULL;
    
    HASH_FIND(hh, tabla->items, &key, sizeof(struct i_key_t), item);
    assert_no_null(item);
    
    HASH_DEL(tabla->items, item);
    FREE_PP(&item, struct csmhashtb2id_item_t);
}

// ------------------------------------------------------------------------------------------

void csmhashtb2id_nousar_clear(struct csmhashtb2id_t *tabla, csmhashtb_FPtr_free_item_ptr func_free_item_ptr)
{
    assert_no_null(tabla);
    
    if (tabla->items != NULL)
    {
        struct csmhashtb2id_item_t *current_item, *tmp;

        HASH_ITER(hh, tabla->items, current_item, tmp)
        {
            HASH_DEL(tabla->items, current_item);
            i_destruye_item(&current_item, func_free_item_ptr);
        }
    }
}

// ------------------------------------------------------------------------------------------

void *csmhashtb2id_nousar_ptr_for_id(struct csmhashtb2id_t *tabla, unsigned long id1, unsigned long id2)
{
    struct i_key_t key;
    struct csmhashtb2id_item_t *item;
    
    assert_no_null(tabla);
    
    key.id1 = id1;
    key.id2 = id2;
    item = NULL;
    
    HASH_FIND(hh, tabla->items, &key, sizeof(struct i_key_t), item);
    assert_no_null(item);
    
    return item->ptr;
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmhashtb2id_nousar_contains_id(struct csmhashtb2id_t *tabla, unsigned long id1, unsigned long id2, void **ptr_opc)
{
    struct i_key_t key;
    struct csmhashtb2id_item_t *item;
    
    assert_no_null(tabla);
    
    key.id1 = id1;
    key.id2 = id2;
    item = NULL;
    
    HASH_FIND(hh, tabla->items, &key, sizeof(struct i_key_t), item);
    
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

CONSTRUCTOR(static struct csmhashtb2id_iterator_t *, i_create_iterator, (struct csmhashtb2id_item_t *next_item))
{
    struct csmhashtb2id_iterator_t *iterator;
    
    iterator = MALLOC(struct csmhashtb2id_iterator_t);
    
    iterator->next_item = next_item;
    
    return iterator;
}

// ------------------------------------------------------------------------------------------

struct csmhashtb2id_iterator_t *csmhashtb2id_nousar_create_iterator(struct csmhashtb2id_t *tabla)
{
    assert_no_null(tabla);
    return i_create_iterator(tabla->items);
}

// ------------------------------------------------------------------------------------------

void csmhashtb2id_nousar_free_iterator(struct csmhashtb2id_iterator_t **iterator)
{
    assert_no_null(iterator);
    assert_no_null(*iterator);
    
    FREE_PP(iterator, struct csmhashtb2id_iterator_t);
}

// ------------------------------------------------------------------------------------------

CSMBOOL csmhashtb2id_nousar_has_next(const struct csmhashtb2id_iterator_t *iterator)
{
    assert_no_null(iterator);
    return IS_TRUE(iterator->next_item != NULL);
}

// ------------------------------------------------------------------------------------------

void csmhashtb2id_nousar_next_pair(struct csmhashtb2id_iterator_t *iterator, unsigned long *id1_opc, unsigned long *id2_opc, void **ptr_opc)
{
    assert_no_null(iterator);
    assert_no_null(iterator->next_item);
    
    ASSIGN_OPTIONAL_VALUE(id1_opc, iterator->next_item->key.id1);
    ASSIGN_OPTIONAL_VALUE(id2_opc, iterator->next_item->key.id2);
    ASSIGN_OPTIONAL_VALUE(ptr_opc, iterator->next_item->ptr);
    
    iterator->next_item = iterator->next_item->hh.next;
}

// ------------------------------------------------------------------------------------------

csmArrayStruct(csmhashtb_item_ptr_t) *csmhashtb2id_nousar_convert_to_array(struct csmhashtb2id_t *tabla)
{
    csmArrayStruct(csmhashtb_item_ptr_t) *tabla_as_array;
    unsigned long idx, count;
    struct csmhashtb2id_item_t *iterator;
    
    assert_no_null(tabla);
    
    iterator = tabla->items;
    count = HASH_COUNT(tabla->items);
    
    tabla_as_array = csmarrayc_new_st_array(count, csmhashtb_item_ptr_t);
    idx = 0;
    
    while (iterator != NULL)
    {
        csmarrayc_set_st(tabla_as_array, idx, iterator->ptr, csmhashtb_item_ptr_t);
        
        iterator = iterator->hh.next;
        idx++;
    }
    
    assert(idx == count);
    
    return tabla_as_array;
}

/*lint -restore*/
