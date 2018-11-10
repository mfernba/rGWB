//
//  csmoctree.c
//  rGWB
//
//  Created by Manuel Fernandez on 10/11/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmoctree.h"

#include "csmarrayc.h"
#include "csmassert.inl"
#include "csmbbox.inl"
#include "csmmem.inl"

csmArrayStruct(csmoctree_element_t);
csmArrayStruct(i_cell_t);

struct csmoctree_element_t
{
    const struct csmoctree_item_t *octree_item;
    
    csmoctree_FPtr_get_extension func_get_extension;
    csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox;
    
    struct csmbbox_t *computed_bbox;
};

struct i_cell_t
{
    struct csmbbox_t *bbox;
    
    csmArrayStruct(csmoctree_element_t) *elements;
    csmArrayStruct(i_cell_t) *cells;
};

struct csmoctree_t
{
    unsigned long max_occupancy;
    csmArrayStruct(csmoctree_element_t) *pendant_elements;
    
    struct i_cell_t *root;
};

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmoctree_element_t *, i_new_element, (
                        const struct csmoctree_item_t *octree_item,
                        csmoctree_FPtr_get_extension func_get_extension,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox,
                        struct csmbbox_t **computed_bbox))
{
    struct csmoctree_element_t *element;
    
    element = MALLOC(struct csmoctree_element_t);
    
    element->octree_item = octree_item;
    
    element->func_get_extension = func_get_extension;
    element->func_intersects_with_bbox = func_intersects_with_bbox;
    
    element->computed_bbox = ASSIGN_POINTER_PP_NOT_NULL(computed_bbox, struct csmbbox_t);
    
    return element;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_element(struct csmoctree_element_t **element)
{
    assert_no_null(element);
    assert_no_null(*element);
    
    csmbbox_free(&(*element)->computed_bbox);
    
    FREE_PP(element, struct csmoctree_element_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmoctree_element_t *, i_new_element_computing_bbox, (
                        const struct csmoctree_item_t *octree_item,
                        csmoctree_FPtr_get_extension func_get_extension,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox))
{
    struct csmbbox_t *computed_bbox;
    
    assert_no_null(func_get_extension);
    
    computed_bbox = func_get_extension(octree_item);
    return i_new_element(octree_item, func_get_extension, func_intersects_with_bbox, &computed_bbox);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_cell_t *, i_new_cell, (
                        struct csmbbox_t **bbox,
                        csmArrayStruct(csmoctree_element_t) **elements,
                        csmArrayStruct(i_cell_t) **cells))
{
    struct i_cell_t *cell;
    
    cell = MALLOC(struct i_cell_t);
    
    cell->bbox = ASSIGN_POINTER_PP_NOT_NULL(bbox, struct csmbbox_t);
    
    cell->elements = ASSIGN_POINTER_PP_NOT_NULL(elements, csmArrayStruct(csmoctree_element_t));
    cell->cells = ASSIGN_POINTER_PP(cells, csmArrayStruct(i_cell_t));
    
    return cell;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_cell(struct i_cell_t **cell)
{
    assert_no_null(cell);
    assert_no_null(*cell);
    
    csmbbox_free(&(*cell)->bbox);
    
    if ((*cell)->elements != NULL)
        csmarrayc_free_st(&(*cell)->elements, csmoctree_element_t, i_free_element);
    
    if ((*cell)->cells != NULL)
        csmarrayc_free_st(&(*cell)->cells, i_cell_t, i_free_cell);
    
    FREE_PP(cell, struct i_cell_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_cell_t *, i_new_root_cell, (struct csmbbox_t **bbox))
{
    csmArrayStruct(csmoctree_element_t) *elements;
    csmArrayStruct(i_cell_t) *cells;
    
    elements = csmarrayc_new_st_array(0, csmoctree_element_t);
    cells = NULL;
    
    return i_new_cell(bbox, &elements, &cells);
}

// ----------------------------------------------------------------------------------------------------

static void i_subdivision_limits(
                        double bbox_start, double bbox_end,
                        unsigned long i, unsigned long no_subdivisions,
                        double *start, double *end)
{
    double size;
    
    assert(bbox_start < bbox_end);
    assert(no_subdivisions > 0);
    assert_no_null(start);
    assert_no_null(end);
    
    size = (bbox_end - bbox_start) / no_subdivisions;
    
    *start = (i == 0) ? bbox_start : bbox_start + i * size;
    *end = (i == no_subdivisions - 1) ? bbox_end : bbox_start + (i + 1) * size;
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static csmArrayStruct(i_cell_t) *, i_explode_cell_bbox, (const struct csmbbox_t *bbox))
{
    csmArrayStruct(i_cell_t) *cells;
    unsigned long i, no_subdivisions;
    double x_min, y_min, z_min, x_max, y_max, z_max;
    unsigned long current_cell_idx;
    
    no_subdivisions = 2;
    cells = csmarrayc_new_st_array(no_subdivisions * no_subdivisions * no_subdivisions, i_cell_t);
    
    csmbbox_get_extension_real(bbox, &x_min, &y_min, &z_min, &x_max, &y_max, &z_max);
    current_cell_idx = 0;
    
    for (i = 0; i < no_subdivisions; i++)
    {
        double x_start, x_end;
        unsigned long j;
        
        i_subdivision_limits(x_min, x_max, i, no_subdivisions, &x_start, &x_end);
        
        for (j = 0; j < no_subdivisions; j++)
        {
            double y_start, y_end;
            unsigned long k;

            i_subdivision_limits(y_min, y_max, j, no_subdivisions, &y_start, &y_end);
            
            for (k = 0; k < no_subdivisions; k++)
            {
                double z_start, z_end;
                struct csmbbox_t *cell_bbox;
                struct i_cell_t *cell;

                i_subdivision_limits(z_min, z_max, j, no_subdivisions, &z_start, &z_end);
                
                cell_bbox = csmbbox_create_empty_box();
                csmbbox_maximize_coord(cell_bbox, x_start, y_start, z_start);
                csmbbox_maximize_coord(cell_bbox, x_end, y_end, z_end);
                
                cell = i_new_root_cell(&cell_bbox);
                csmarrayc_insert_element_st(cells, current_cell_idx, cell, i_cell_t);
                
                current_cell_idx++;
            }
        }
    }
    
    return cells;
}

// ----------------------------------------------------------------------------------------------------

static void i_append_element_to_cell(
                        struct i_cell_t *cell,
                        unsigned long max_occupancy,
                        const struct csmoctree_element_t *element,
                        CSMBOOL *added)
{
    CSMBOOL added_loc;
    
    assert_no_null(cell);
    assert(max_occupancy > 0);
    assert_no_null(added);
    
    if (csmbbox_intersects_with_other_bbox(element->computed_bbox, cell->bbox) == CSMFALSE)
    {
        added_loc = CSMFALSE;
    }
    else
    {
        if (cell->cells == NULL)
        {
            CSMBOOL intersects;
            struct csmoctree_element_t *element_copy;
            unsigned long current_occupancy;
            
            intersects = element->func_intersects_with_bbox(element->octree_item, cell->bbox);
            assert(intersects == CSMTRUE);
            
            element_copy = i_new_element_computing_bbox(element->octree_item, element->func_get_extension, element->func_intersects_with_bbox);
            csmarrayc_append_element_st(cell->elements, element_copy, csmoctree_element_t);
                
            added_loc = CSMTRUE;
            
            current_occupancy = csmarrayc_count_st(cell->elements, csmoctree_element_t);
            
            if (current_occupancy > max_occupancy)
            {
                unsigned long i;
                unsigned long no_cells;
                
                cell->cells = i_explode_cell_bbox(cell->bbox);
                
                no_cells = csmarrayc_count_st(cell->cells, i_cell_t);
                assert(no_cells > 0);
                
                for (i = 0; i < current_occupancy; i++)
                {
                    const struct csmoctree_element_t *element;
                    CSMBOOL added_i;
                    unsigned long j;
                    
                    element = csmarrayc_get_const_st(cell->elements, i, csmoctree_element_t);
                    added_i = CSMFALSE;
                    
                    for (j = 0; j < no_cells; j++)
                    {
                        struct i_cell_t *inner_cell;
                        CSMBOOL added_in_inner_cell;
                        
                        inner_cell = csmarrayc_get_st(cell->cells, j, i_cell_t);
                        i_append_element_to_cell(inner_cell, max_occupancy, element, &added_in_inner_cell);
                        
                        if (added_in_inner_cell == CSMTRUE)
                            added_i = CSMTRUE;
                    }
                    
                    assert(added_i == CSMTRUE);
                    csmarrayc_free_st(&cell->elements, csmoctree_element_t, i_free_element);
                }
            }
        }
        else
        {
            unsigned long i, no_cells;
            
            assert(csmarrayc_count_st(cell->elements, csmoctree_element_t) == 0);
            no_cells = csmarrayc_count_st(cell->cells, i_cell_t);
            
            added_loc = CSMFALSE;
            
            for (i = 0; i < no_cells; i++)
            {
                struct i_cell_t *inner_cell;
                CSMBOOL added_in_inner_cell;
                
                inner_cell = csmarrayc_get_st(cell->cells, i, i_cell_t);
                i_append_element_to_cell(inner_cell, max_occupancy, element, &added_in_inner_cell);
                
                if (added_in_inner_cell == CSMTRUE)
                    added_loc = CSMTRUE;
            }
        }
    }
    
    *added = added_loc;
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmoctree_t *, i_new, (
                        unsigned long max_occupancy,
                        csmArrayStruct(csmoctree_element_t) **elements,
                        struct i_cell_t **root))
{
    struct csmoctree_t *octree;
    
    octree = MALLOC(struct csmoctree_t);
    
    octree->max_occupancy = max_occupancy;
    octree->pendant_elements = ASSIGN_POINTER_PP_NOT_NULL(elements, csmArrayStruct(csmoctree_element_t));
    
    octree->root = ASSIGN_POINTER_PP(root, struct i_cell_t);
    
    return octree;
}
            
// ----------------------------------------------------------------------------------------------------

struct csmoctree_t *csmoctree_new(unsigned long max_occupancy)
{
    csmArrayStruct(csmoctree_element_t) *pendant_elements;
    struct i_cell_t *root;
    
    pendant_elements = csmarrayc_new_st_array(0, csmoctree_element_t);
    root = NULL;

    return i_new(max_occupancy, &pendant_elements, &root);
}

// ----------------------------------------------------------------------------------------------------

void csmoctree_free(struct csmoctree_t **octree)
{
    assert_no_null(octree);
    assert_no_null(*octree);
    
    csmarrayc_free_st(&(*octree)->pendant_elements, csmoctree_element_t, i_free_element);
    
    if ((*octree)->root != NULL)
        i_free_cell(&(*octree)->root);
    
    FREE_PP(octree, struct csmoctree_t);
}

// ----------------------------------------------------------------------------------------------------

void csmoctree_dontuse_append_item(
                        struct csmoctree_t *octree,
                        const struct csmoctree_item_t *octree_item,
                        csmoctree_FPtr_get_extension func_get_extension,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox)
{
    struct csmoctree_element_t *element;
    
    assert_no_null(octree);
    
    element = i_new_element_computing_bbox(octree_item, func_get_extension, func_intersects_with_bbox);
    csmarrayc_append_element_st(octree->pendant_elements, element, csmoctree_element_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmbbox_t *, i_compute_elements_bbox, (const csmArrayStruct(csmoctree_element_t) *elements))
{
    struct csmbbox_t *bbox;
    unsigned long i, no_elems;
    
    no_elems = csmarrayc_count_st(elements, csmoctree_element_t);
    assert(no_elems > 0);
    
    bbox = csmbbox_create_empty_box();
    
    for (i = 0; i < no_elems; i++)
    {
        const struct csmoctree_element_t *element;
        
        element = csmarrayc_get_const_st(elements, i, csmoctree_element_t);
        assert_no_null(element);
        
        csmbbox_maximize_bbox(bbox, element->computed_bbox);
    }
    
    return bbox;
}

// ----------------------------------------------------------------------------------------------------

void csmoctree_build(struct csmoctree_t *octree)
{
    struct csmbbox_t *elements_bbox;
    unsigned long i, no_pendant_elements;
    
    assert_no_null(octree);
    no_pendant_elements = csmarrayc_count_st(octree->pendant_elements, csmoctree_element_t);
    assert(no_pendant_elements > 0);
    assert(octree->root == NULL);
    
    elements_bbox = i_compute_elements_bbox(octree->pendant_elements);
    octree->root = i_new_root_cell(&elements_bbox);
    
    for (i = 0; i < no_pendant_elements; i++)
    {
        const struct csmoctree_element_t *element;
        CSMBOOL added;
        
        element = csmarrayc_get_const_st(octree->pendant_elements, i, csmoctree_element_t);
        
        i_append_element_to_cell(octree->root, octree->max_occupancy, element, &added);
        assert(added == CSMTRUE);
    };
    
    csmarrayc_free_st(&octree->pendant_elements, csmoctree_element_t, NULL);
}







