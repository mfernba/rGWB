//
//  csmtest_array.c
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmtest_array.inl"

#include "csmarrayc.inl"
#include "csmassert.inl"
#include "csmmem.inl"
#include "csmmath.inl"

csmArrayStruct(i_element_t);

struct i_element_t
{
    unsigned long element_id;
    double value2;
};

struct i_element_t *i_element_new(unsigned long element_id, double value2)
{
    struct i_element_t *element;
    
    element = MALLOC(struct i_element_t);
    
    element->element_id = element_id;
    element->value2 = value2;
    
    return element;
}

struct i_element_t *i_element_copy(const struct i_element_t *element)
{
    assert_no_null(element);
    return i_element_new(element->element_id, element->value2);
}

static void i_free_element(struct i_element_t **element)
{
    FREE_PP(element, struct i_element_t);
}

static enum csmcompare_t i_compare_element(const struct i_element_t *element1, const struct i_element_t *element2)
{
    if (element1->element_id == element2->element_id)
        return CSMCOMPARE_EQUAL;
    else if (element1->element_id < element2->element_id)
        return CSMCOMPARE_FIRST_LESS;
    else
        return CSMCOMPARE_FIRST_GREATER;
}

static void i_append_element(csmArrayStruct(i_element_t) *elements, unsigned long element_id, double value)
{
    struct i_element_t *element;
    
    element = i_element_new(element_id, value);
    csmarrayc_append_element_st(elements, element, i_element_t);
}

static void i_set_element(csmArrayStruct(i_element_t) *elements, unsigned long idx, unsigned long element_id, double value)
{
    struct i_element_t *element;
    
    element = i_element_new(element_id, value);
    csmarrayc_set_st(elements, idx, element, i_element_t);
}

static void i_print_array_elements(csmArrayStruct(i_element_t) *elements)
{
    unsigned long i, num_elements;
    
    num_elements = csmarrayc_count_st(elements, i_element_t);
    
    if (num_elements == 0)
        fprintf(stdout, "Empty array\n");
    else
        fprintf(stdout, "Array:\n");
    
    for (i = 0; i < num_elements; i++)
    {
        const struct i_element_t *element;
        
        element = csmarrayc_get_st(elements, i, i_element_t);
        fprintf(stdout, "E: %lu, %lf\n", element->element_id, element->value2);
    }
}

static enum csmcompare_t i_compare_elements(const struct i_element_t *e1, const struct i_element_t *e2)
{
    assert_no_null(e1);
    assert_no_null(e2);
    
    if (e1->element_id == e2->element_id)
        return CSMCOMPARE_EQUAL;
    else if (e1->element_id < e2->element_id)
        return CSMCOMPARE_FIRST_LESS;
    else
        return CSMCOMPARE_FIRST_GREATER;
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_is_element_with_id(const struct i_element_t *e1, const unsigned long *searched_id)
{
    assert_no_null(e1);
    assert_no_null(searched_id);
    
    return IS_TRUE(e1->element_id == *searched_id);
}

// ------------------------------------------------------------------------------------------

void csmtest_array_test1(void)
{
    csmArrayStruct(i_element_t) *elements;
    unsigned long searched_id;
    
    {
        elements = csmarrayc_new_st_array(0, i_element_t);
        i_append_element(elements, 5, 2.5);
        i_append_element(elements, 3, 33.);
        i_append_element(elements, 7, 23.11);
        i_append_element(elements, 6, 1.2);
        
        fprintf(stdout, "Initial array: \n");
        i_print_array_elements(elements);
        
        csmarrayc_qsort_st(elements, i_element_t, i_compare_elements);
        fprintf(stdout, "After qsort: \n");
        i_print_array_elements(elements);
        
        searched_id = 7;
        
        if (csmarrayc_contains_element_st(elements, i_element_t, &searched_id, unsigned long, i_is_element_with_id, NULL) == CSMTRUE)
            fprintf(stdout, "Containts element %lu\n", searched_id);
        else
            fprintf(stdout, "Doesn't containts element %lu\n", searched_id);

        fprintf(stdout, "Deleting idx 1, 2: \n");
        csmarrayc_delete_element_st(elements, 1, i_element_t, i_free_element);
        i_print_array_elements(elements);
        csmarrayc_delete_element_st(elements, 1, i_element_t, i_free_element);
        i_print_array_elements(elements);

        searched_id = 5;
        
        if (csmarrayc_contains_element_st(elements, i_element_t, &searched_id, unsigned long, i_is_element_with_id, NULL) == CSMTRUE)
            fprintf(stdout, "Containts element %lu\n", searched_id);
        else
            fprintf(stdout, "Doesn't containts element %lu\n", searched_id);

        csmarrayc_delete_element_st(elements, 0, i_element_t, i_free_element);
        csmarrayc_delete_element_st(elements, 0, i_element_t, i_free_element);
        i_print_array_elements(elements);
        
        csmarrayc_free_st(&elements, i_element_t, i_free_element);
    }

    {
        elements = csmarrayc_new_st_array(2, i_element_t);
        i_set_element(elements, 1, 300, 1.25);
        i_set_element(elements, 0, 100, 33.25);
        i_print_array_elements(elements);
        
        i_append_element(elements, 6, 1.2);
        i_append_element(elements, 7, 1.2);
        i_print_array_elements(elements);
        
        csmarrayc_free_st(&elements, i_element_t, i_free_element);
    }
    
    {
        csmArrayStruct(i_element_t) *elements_copy;

        elements = csmarrayc_new_st_array(2, i_element_t);
        i_set_element(elements, 1, 300, 1.25);
        i_set_element(elements, 0, 100, 33.25);
        i_print_array_elements(elements);
        
        i_append_element(elements, 6, 1.2);
        i_append_element(elements, 7, 1.2);
        i_print_array_elements(elements);
        
        elements_copy = csmarrayc_copy_st_array(elements, i_element_t, i_element_copy);
        i_print_array_elements(elements_copy);
        
        csmarrayc_invert(elements_copy, i_element_t);
        i_print_array_elements(elements_copy);
        
        csmarrayc_free_st(&elements, i_element_t, i_free_element);
        csmarrayc_free_st(&elements_copy, i_element_t, i_free_element);
    }
}

// ------------------------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

static void i_append_element2(void *ptr_datos, struct i_element_t *e, size_t element_size)
{
    memcpy(ptr_datos, &e, element_size);
}

static void *i_make_array(size_t *element_size)
{
    void *ptr_datos;
    struct i_element_t *e1, *e2;
    
    ptr_datos = (void *)malloc(sizeof(struct i_element_t *) * 2);
    *element_size = sizeof(struct i_element_t *);
    
    e1 = i_element_new(100, 220.0);
    i_append_element2(ptr_datos, e1, *element_size);
    
    e2 = i_element_new(200, 220.0);
    i_append_element2(ptr_datos + *element_size, e2, *element_size);
    
    return ptr_datos;
}

void csmtest_array_test2(void)
{
    void *ptr_datos;
    size_t element_size;
    struct i_element_t *ve1, *ve2;
    
    ptr_datos = i_make_array(&element_size);
    
    ve1 = *((struct i_element_t **)(ptr_datos));
    ve2 = *((struct i_element_t **)(ptr_datos + element_size));
    
    free(ptr_datos);
}
