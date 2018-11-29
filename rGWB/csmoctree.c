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
#include "csmstring.inl"

csmArrayStruct(csmoctree_element_t);
csmArrayStruct(i_cell_t);

struct i_cell_t
{
    struct csmbbox_t *bbox;
    
    const csmArrayStruct(csmoctree_item_t) *elements;
    csmArrayStruct(i_cell_t) *cells;
};

struct csmoctree_t
{
    unsigned long max_occupancy;
    struct csmbbox_t *octree_bbox;

    csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox;
    
    const csmArrayStruct(csmoctree_item_t) *pendant_elements;

    struct i_cell_t *root;
};

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_cell_t *, i_new_cell, (
                        struct csmbbox_t **bbox,
                        const csmArrayStruct(csmoctree_item_t) **elements,
                        csmArrayStruct(i_cell_t) **cells))
{
    struct i_cell_t *cell;
    
    cell = MALLOC(struct i_cell_t);
    
    cell->bbox = ASSIGN_POINTER_PP_NOT_NULL(bbox, struct csmbbox_t);
    
    cell->elements = ASSIGN_POINTER_PP_NOT_NULL(elements, const csmArrayStruct(csmoctree_item_t));
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
        csmarrayc_free_const_st(&(*cell)->elements, csmoctree_item_t);
    
    if ((*cell)->cells != NULL)
        csmarrayc_free_st(&(*cell)->cells, i_cell_t, i_free_cell);
    
    FREE_PP(cell, struct i_cell_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_cell_t *, i_new_root_cell, (struct csmbbox_t **bbox))
{
    const csmArrayStruct(csmoctree_item_t) *elements;
    csmArrayStruct(i_cell_t) *cells;
    
    elements = csmarrayc_new_const_st_array(0, csmoctree_item_t);
    cells = NULL;
    
    return i_new_cell(bbox, &elements, &cells);
}

// ----------------------------------------------------------------------------------------------------

static void i_set_cell(
                        unsigned long cell_idx,
                        double x_min, double y_min, double z_min, double x_max, double y_max, double z_max,
                        csmArrayStruct(i_cell_t) *cells)
{
    struct csmbbox_t *cell_bbox;
    struct i_cell_t *cell;
    
    assert(x_min < x_max);
    assert(y_min < y_max);
    assert(z_min < z_max);

    cell_bbox = csmbbox_create_empty_box();
    csmbbox_maximize_coord(cell_bbox, x_min, y_min, z_min);
    csmbbox_maximize_coord(cell_bbox, x_max, y_max, z_max);
    csmbbox_compute_bsphere_and_margins(cell_bbox);
    
    cell = i_new_root_cell(&cell_bbox);
    csmarrayc_set_st(cells, cell_idx, cell, i_cell_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static csmArrayStruct(i_cell_t) *, i_explode_cell_bbox, (const struct csmbbox_t *bbox))
{
    csmArrayStruct(i_cell_t) *cells;
    double x_min, y_min, z_min, x_max, y_max, z_max;
    double x_middle, y_middle, z_middle;
    unsigned long cell_idx;

    csmbbox_get_extension_real(bbox, &x_min, &y_min, &z_min, &x_max, &y_max, &z_max);
    
    x_middle = 0.5 * (x_min + x_max);
    y_middle = 0.5 * (y_min + y_max);
    z_middle = 0.5 * (z_min + z_max);
    
    cells = csmarrayc_new_st_array(8, i_cell_t);
    cell_idx = 0;
    
    i_set_cell(cell_idx++, x_min, y_min, z_min, x_middle, y_middle, z_middle, cells);
    i_set_cell(cell_idx++, x_middle, y_min, z_min, x_max, y_middle, z_middle, cells);
    i_set_cell(cell_idx++, x_min, y_middle, z_min, x_middle, y_max, z_middle, cells);
    i_set_cell(cell_idx++, x_middle, y_middle, z_min, x_max, y_max, z_middle, cells);
    
    i_set_cell(cell_idx++, x_min, y_min, z_middle, x_middle, y_middle, z_max, cells);
    i_set_cell(cell_idx++, x_middle, y_min, z_middle, x_max, y_middle, z_max, cells);
    i_set_cell(cell_idx++, x_min, y_middle, z_middle, x_middle, y_max, z_max, cells);
    i_set_cell(cell_idx++, x_middle, y_middle, z_middle, x_max, y_max, z_max, cells);
    
    return cells;
}

// ----------------------------------------------------------------------------------------------------

static void i_append_element_to_cell(
                        unsigned long recursion_level,
                        struct i_cell_t *cell,
                        unsigned long max_occupancy,
                        const struct csmoctree_item_t *element,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox,
                        CSMBOOL *added)
{
    CSMBOOL added_loc;
    
    assert_no_null(cell);
    assert(recursion_level < 10000);
    assert(max_occupancy > 0);
    assert_no_null(func_intersects_with_bbox);
    assert_no_null(added);
    
    if (func_intersects_with_bbox(element, cell->bbox) == CSMFALSE)
    {
        added_loc = CSMFALSE;
    }
    else
    {
        if (cell->cells == NULL)
        {
            unsigned long current_occupancy;
            
            csmarrayc_append_element_st(cell->elements, element, csmoctree_item_t);
                
            added_loc = CSMTRUE;
            
            current_occupancy = csmarrayc_count_st(cell->elements, csmoctree_item_t);
            
            if (current_occupancy > max_occupancy)
            {
                unsigned long i;
                unsigned long no_cells;
                
                cell->cells = i_explode_cell_bbox(cell->bbox);
                
                no_cells = csmarrayc_count_st(cell->cells, i_cell_t);
                assert(no_cells == 8);
                
                for (i = 0; i < current_occupancy; i++)
                {
                    const struct csmoctree_item_t *element;
                    CSMBOOL added_i;
                    unsigned long j;
                    
                    element = csmarrayc_get_const_st(cell->elements, i, csmoctree_item_t);
                    added_i = CSMFALSE;
                    
                    for (j = 0; j < no_cells; j++)
                    {
                        struct i_cell_t *inner_cell;
                        CSMBOOL added_in_inner_cell;
                        
                        inner_cell = csmarrayc_get_st(cell->cells, j, i_cell_t);
                        i_append_element_to_cell(recursion_level + 1, inner_cell, max_occupancy, element, func_intersects_with_bbox, &added_in_inner_cell);
                        
                        if (added_in_inner_cell == CSMTRUE)
                            added_i = CSMTRUE;
                    }
                    
                    assert(added_i == CSMTRUE);
                }
                
                csmarrayc_free_const_st(&cell->elements, csmoctree_item_t);
            }
        }
        else
        {
            unsigned long i, no_cells;
            
            assert(cell->elements == NULL);
            
            no_cells = csmarrayc_count_st(cell->cells, i_cell_t);
            assert(no_cells == 8);
            
            added_loc = CSMFALSE;
            
            for (i = 0; i < no_cells; i++)
            {
                struct i_cell_t *inner_cell;
                CSMBOOL added_in_inner_cell;
                
                inner_cell = csmarrayc_get_st(cell->cells, i, i_cell_t);
                i_append_element_to_cell(recursion_level + 1, inner_cell, max_occupancy, element, func_intersects_with_bbox, &added_in_inner_cell);
                
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
                        struct csmbbox_t **octree_bbox,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox,
                        const csmArrayStruct(csmoctree_item_t) **elements,
                        struct i_cell_t **root))
{
    struct csmoctree_t *octree;
    
    octree = MALLOC(struct csmoctree_t);
    
    octree->max_occupancy = max_occupancy;
    octree->octree_bbox = ASSIGN_POINTER_PP_NOT_NULL(octree_bbox, struct csmbbox_t);
    
    octree->func_intersects_with_bbox = func_intersects_with_bbox;
    
    octree->pendant_elements = ASSIGN_POINTER_PP_NOT_NULL(elements, const csmArrayStruct(csmoctree_item_t));
    
    octree->root = ASSIGN_POINTER_PP(root, struct i_cell_t);
    
    return octree;
}
            
// ----------------------------------------------------------------------------------------------------

struct csmoctree_t *csmoctree_dontuse_new(
                        unsigned long max_occupancy,
                        struct csmbbox_t **octree_bbox,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox)
{
    const csmArrayStruct(csmoctree_item_t) *pendant_elements;
    struct i_cell_t *root;
    
    pendant_elements = csmarrayc_new_const_st_array(0, csmoctree_item_t);
    root = NULL;

    return i_new(max_occupancy, octree_bbox, func_intersects_with_bbox, &pendant_elements, &root);
}

// ----------------------------------------------------------------------------------------------------

void csmoctree_dontuse_free(struct csmoctree_t **octree)
{
    assert_no_null(octree);
    assert_no_null(*octree);
    
    csmarrayc_free_const_st(&(*octree)->pendant_elements, csmoctree_item_t);
    
    csmbbox_free(&(*octree)->octree_bbox);
    
    if ((*octree)->root != NULL)
        i_free_cell(&(*octree)->root);
    
    FREE_PP(octree, struct csmoctree_t);
}

// ----------------------------------------------------------------------------------------------------

void csmoctree_dontuse_append_item(struct csmoctree_t *octree, const struct csmoctree_item_t *octree_item)
{
    assert_no_null(octree);
    csmarrayc_append_element_const_st(octree->pendant_elements, octree_item, csmoctree_item_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_update_octree(struct csmoctree_t *octree)
{
    unsigned long no_pendant_elements;
    
    assert_no_null(octree);
    
    if (octree->root == NULL)
    {
        struct csmbbox_t *root_bbox;
        
        root_bbox = csmbbox_copy(octree->octree_bbox);
        octree->root = i_new_root_cell(&root_bbox);
    }
    
    no_pendant_elements = csmarrayc_count_st(octree->pendant_elements, csmoctree_item_t);
    
    if (no_pendant_elements > 0)
    {
        unsigned long i;
        
        for (i = 0; i < no_pendant_elements; i++)
        {
            const struct csmoctree_item_t *element;
            CSMBOOL added;
            
            element = csmarrayc_get_const_st(octree->pendant_elements, i, csmoctree_item_t);
            
            i_append_element_to_cell(0, octree->root, octree->max_occupancy, element, octree->func_intersects_with_bbox, &added);
            assert(added == CSMTRUE);
        };
        
        csmarrayc_free_const_st(&octree->pendant_elements, csmoctree_item_t);
        octree->pendant_elements = csmarrayc_new_const_st_array(0, csmoctree_item_t);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_append_bbox_neighbors(
                        unsigned long recursion_level,
                        const struct i_cell_t *cell,
                        const struct csmbbox_t *bbox,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox,
                        const csmArrayStruct(csmoctree_item_t) *bbox_neighbors)
{
    assert_no_null(cell);
    assert(recursion_level < 10000);
    assert_no_null(func_intersects_with_bbox);
    
    if (csmbbox_intersects_with_other_bbox(cell->bbox, bbox) == CSMTRUE)
    {
        if (cell->cells == NULL)
        {
            unsigned long i, no_elements;
            
            no_elements = csmarrayc_count_st(cell->elements, csmoctree_item_t);
            
            for (i = 0; i < no_elements; i++)
            {
                const struct csmoctree_item_t *element;
                
                element = csmarrayc_get_const_st(cell->elements, i, csmoctree_item_t);
                
                if (func_intersects_with_bbox(element, bbox) == CSMTRUE)
                    csmarrayc_append_element_const_st(bbox_neighbors, element, csmoctree_item_t);
            }
        }
        else
        {
            unsigned long i, no_cells;
            
            assert(cell->elements == NULL);
            
            no_cells = csmarrayc_count_st(cell->cells, i_cell_t);
            assert(no_cells == 8);
            
            for (i = 0; i < no_cells; i++)
            {
                struct i_cell_t *inner_cell;
                
                inner_cell = csmarrayc_get_st(cell->cells, i, i_cell_t);
                i_append_bbox_neighbors(recursion_level + 1, inner_cell, bbox, func_intersects_with_bbox, bbox_neighbors);
            }
        }
    }
}

// ----------------------------------------------------------------------------------------------------

const csmArrayStruct(csmoctree_item_t) *csmoctree_dontuse_get_bbox_neighbors(struct csmoctree_t *octree, const struct csmbbox_t *bbox)
{
    const csmArrayStruct(csmoctree_item_t) *bbox_neighbors;
    
    assert_no_null(octree);
    
    i_update_octree(octree);
    
    bbox_neighbors = csmarrayc_new_const_st_array(0, csmoctree_item_t);
    i_append_bbox_neighbors(0, octree->root, bbox, octree->func_intersects_with_bbox, bbox_neighbors);

    return bbox_neighbors;
}
