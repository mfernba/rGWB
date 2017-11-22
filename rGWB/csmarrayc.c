// Generic container...

#include "csmarrayc.inl"

#include "csmassert.inl"
#include "csmarqsort.inl"
#include "csmmem.inl"
#include <string.h>

static const char i_DEBUG_MASK = 0xFA;
static const size_t i_INITIAL_CAPACITY = 100;

struct csmarrayc_t
{
    char debug_mask;
    
    CSMBOOL is_pointer_array;
    size_t no_elems;
    size_t capacity;
    
    void *ptr_data;
    size_t element_data_size;
};

struct i_cmp_data_t
{
    struct csmarrayc_t *array;
    csmarrayc_FPtr_compare func_compare;
};

struct i_cmp_data_1_extra_t
{
    struct csmarrayc_t *array;
    struct csmarrayc_extra_item_t *extra_item;
    csmarrayc_FPtr_compare_1_extra func_compare;
};

// ---------------------------------------------------------------------------------

static void i_integrity(const struct csmarrayc_t *array)
{
    assert_no_null(array);
    assert(array->no_elems <= array->capacity);
    assert_no_null(array->ptr_data);
    assert(array->element_data_size > 0);
    assert(array->debug_mask == i_DEBUG_MASK);
}

// ---------------------------------------------------------------------------------

static struct csmarrayc_t *i_new(
                                  unsigned short is_pointer_array,
                                  size_t no_elems,
                                  size_t capacity,
                                  void **ptr_data,
                                  size_t element_data_size)
{
    struct csmarrayc_t *array;
    
    array = MALLOC(struct csmarrayc_t);
    
    array->is_pointer_array = is_pointer_array;
    
    array->no_elems = no_elems;
    array->capacity = capacity;
    array->ptr_data = ASIGNA_PUNTERO_PP_NO_NULL(ptr_data, void);
    array->element_data_size = element_data_size;
    array->debug_mask = i_DEBUG_MASK;
    
    i_integrity(array);
    
    return array;
}

// ---------------------------------------------------------------------------------

struct csmarrayc_t *csmarrayc_dontuse_new_ptr_array(size_t capacity_inicial, size_t element_data_size)
{
    CSMBOOL is_pointer_array;
    size_t no_elems;
    size_t capacity;
    void *ptr_data;
    
    is_pointer_array = CSMTRUE;
    
    if (capacity_inicial == 0)
    {
        no_elems = 0;
        capacity = i_INITIAL_CAPACITY;
    }
    else
    {
        no_elems = capacity_inicial;
        capacity = capacity_inicial;
    }
    
    ptr_data = (void *)malloc(element_data_size * capacity);
    
    return i_new(is_pointer_array, no_elems, capacity, &ptr_data, element_data_size);
}

// ---------------------------------------------------------------------------------

struct csmarrayc_t *csmarrayc_dontuse_copy_ptr_array(const struct csmarrayc_t *array, csmarrayc_FPtr_copy_struct func_copy_element)
{
    size_t i, offset;
    void *ptr_data;
    
    assert_no_null(array);
    assert_no_null(func_copy_element);
    
    ptr_data = (void *)malloc(array->element_data_size * array->no_elems);
    offset = 0;
    
    for (i = 0; i < array->no_elems; i++)
    {
        const void *element;
        void *element_copy;
        
        element = *(void **)(array->ptr_data + offset);
        element_copy = func_copy_element(element);
        
        memcpy(ptr_data + offset, &element_copy, array->element_data_size);
        
        offset += array->element_data_size;
    }
    
    return i_new(array->is_pointer_array, array->no_elems, array->no_elems, &ptr_data, array->element_data_size);
}

// ---------------------------------------------------------------------------------

void csmarrayc_dontuse_free(struct csmarrayc_t **array, csmarrayc_FPtr_free_struct func_free_struct)
{
    assert_no_null(array);
    i_integrity(*array);
    
    if ((*array)->is_pointer_array == CSMTRUE && func_free_struct != NULL)
    {
        void *ptr_data;
        unsigned long i, offset;
        
        ptr_data = (*array)->ptr_data;
        offset = 0;
        
        for (i = 0; i < (*array)->no_elems; i++)
        {
            func_free_struct(((void **)(ptr_data + offset)));
            offset += (*array)->element_data_size;
        }
    }

    FREE_PP(&(*array)->ptr_data, void);
    
    FREE_PP(array, struct csmarrayc_t);
}

// ---------------------------------------------------------------------------------

size_t csmarrayc_dontuse_count(const struct csmarrayc_t *array)
{
    i_integrity(array);
    return array->no_elems;
}

// ---------------------------------------------------------------------------------

void csmarrayc_dontuse_append_element(struct csmarrayc_t *array, void *dato)
{
    i_integrity(array);
    
    if (array->capacity == array->no_elems)
    {
        size_t new_capacity;
        void *ptr_data_extended;
        
        new_capacity = array->capacity + (3 * array->capacity) / 2;
        ptr_data_extended = (void *)malloc(new_capacity * array->element_data_size);
        assert_no_null(ptr_data_extended);
        
        //memset(ptr_data_extended, 0xFF, new_capacity * array->element_data_size);
        memcpy(ptr_data_extended, array->ptr_data, array->no_elems * array->element_data_size);
        
        FREE_PP(&array->ptr_data, void);
        
        array->ptr_data = ASIGNA_PUNTERO_PP_NO_NULL(&ptr_data_extended, void);
        array->capacity = new_capacity;;
    }
    
    memcpy(array->ptr_data + array->no_elems * array->element_data_size, dato, array->element_data_size);
    array->no_elems++;
}

// ---------------------------------------------------------------------------------

void csmarrayc_dontuse_set_element(struct csmarrayc_t *array, unsigned long idx, void *dato)
{
    i_integrity(array);
    assert(idx < array->no_elems);
    
    memcpy(array->ptr_data + idx * array->element_data_size, &dato, array->element_data_size);
}

// ---------------------------------------------------------------------------------

CSMBOOL csmarrayc_dontuse_contains_element(
                        const struct csmarrayc_t *array,
                        const csmarrayc_byte *search_data,
                        csmarrayc_FPtr_match_condition func_match_condition,
                        unsigned long *idx)
{
    CSMBOOL exists_element;
    unsigned long idx_loc;
    unsigned long i;
    unsigned long offset;
    
    assert_no_null(array);
    
    exists_element = CSMFALSE;
    idx_loc = ULONG_MAX;
    
    offset = 0;
    
    for (i = 0; i < array->no_elems; i++)
    {
        const void *element;
        
        element = *(void **)(array->ptr_data + offset);
        
        if (func_match_condition(element, search_data) == CSMTRUE)
        {
            exists_element = CSMTRUE;
            idx_loc = i;
            break;
        }
        
        offset += array->element_data_size;
    }
    
    ASSIGN_OPTIONAL_VALUE(idx, idx_loc);
    
    return exists_element;
}

// ---------------------------------------------------------------------------------

void *csmarrayc_dontuse_get(struct csmarrayc_t *array, unsigned long idx)
{
    unsigned long offset;
    
    assert_no_null(array);
    assert(idx < array->no_elems);
    
    offset = array->element_data_size * idx;
    return (void *)(*(void **)(array->ptr_data + offset));
}

// ---------------------------------------------------------------------------------

void csmarrayc_dontuse_delete_element(struct csmarrayc_t *array, unsigned long idx, csmarrayc_FPtr_free_struct func_free)
{
    unsigned long offset;
    
    assert_no_null(array);
    assert(idx < array->no_elems);
    
    offset = array->element_data_size * idx;
    
    if (func_free != NULL)
        func_free((void **)(array->ptr_data + offset));
    
    if (idx < array->no_elems - 1)
    {
        size_t bytes_a_mover;
        
        bytes_a_mover = (array->no_elems - 1 - idx) * array->element_data_size;
        memmove(array->ptr_data + offset, array->ptr_data + offset + array->element_data_size, bytes_a_mover);
    }
    
    array->no_elems--;
}

// ---------------------------------------------------------------------------------

static int i_cmp_function_ptr(const void *cmp_data_void, const void *e1, const void *e2)
{
    struct i_cmp_data_t *cmp_data;
    
    cmp_data = (struct i_cmp_data_t *)cmp_data_void;
    assert_no_null(cmp_data);
    
    return (int)cmp_data->func_compare(*(void **)e1, *(void **)e2);
}

// ---------------------------------------------------------------------------------

void csmarrayc_dontuse_qsort(struct csmarrayc_t *array, csmarrayc_FPtr_compare func_compare)
{
    struct i_cmp_data_t cmp_data;
    
    assert_no_null(array);
    
    cmp_data.array = array;
    cmp_data.func_compare = func_compare;
    
    csmarqsort(array->ptr_data, array->no_elems, array->element_data_size, (void *)&cmp_data, i_cmp_function_ptr);
}

// ---------------------------------------------------------------------------------

static int i_cmp_function_ptr_1_extra(const void *cmp_data_void, const void *e1, const void *e2)
{
    struct i_cmp_data_1_extra_t *cmp_data;
    
    cmp_data = (struct i_cmp_data_1_extra_t *)cmp_data_void;
    assert_no_null(cmp_data);
    
    return (int)cmp_data->func_compare(*(void **)e1, *(void **)e2, cmp_data->extra_item);
}

// ---------------------------------------------------------------------------------

void csmarrayc_dontuse_qsort_1_extra(
                        struct csmarrayc_t *array,
                        struct csmarrayc_extra_item_t *extra_item,
                        csmarrayc_FPtr_compare_1_extra func_compare_1_extra)
{
    struct i_cmp_data_1_extra_t cmp_data;
    
    assert_no_null(array);
    
    cmp_data.array = array;
    cmp_data.extra_item = extra_item;
    cmp_data.func_compare = func_compare_1_extra;
    
    csmarqsort(array->ptr_data, array->no_elems, array->element_data_size, (void *)&cmp_data, i_cmp_function_ptr_1_extra);
}

// ---------------------------------------------------------------------------

void csmarrayc_dontuse_invert(struct csmarrayc_t *array)
{
	assert_no_null(array);
	
	if (array->no_elems >= 2)
	{
		unsigned long idx1, idx2;
		
		idx1 = 0;
		idx2 = array->no_elems - 1;
		
		while (idx1 < idx2)
		{
            const void *element1, *element2;
            
            element1 = *(void **)(array->ptr_data + idx1 * array->element_data_size);
            element2 = *(void **)(array->ptr_data + idx2 * array->element_data_size);
            
            memcpy(array->ptr_data + idx1 * array->element_data_size, &element2, array->element_data_size);
            memcpy(array->ptr_data + idx2 * array->element_data_size, &element1, array->element_data_size);
			
			idx1++;
			idx2--;
		}
	}
}











