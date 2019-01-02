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
#include "csmdebug.inl"
#include "csmid.inl"
#include "csmmath.tli"
#include "csmmem.inl"
#include "csmstring.inl"

csmArrayStruct(csmoctree_element_t);
csmArrayStruct(i_cell_t);

struct i_cell_item_t
{
    const struct csmoctree_item_t *item;
    unsigned long last_visited_timestamp;
};

struct i_cell_t
{
    unsigned long cell_id;
    
    struct csmbbox_t *bbox;
    struct csmbbox_t *extended_bbox;
    
    const csmArrayStruct(i_cell_item_t) *edge_elements;
    const csmArrayStruct(i_cell_item_t) *interior_elements;
    csmArrayStruct(i_cell_t) *cells;
};

struct csmoctree_t
{
    unsigned long id_new_element;
    
    unsigned long max_occupancy;
    double tolerance_margin;
    
    struct csmbbox_t *octree_bbox;
    double minimun_cell_size;

    csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox;
    
    const csmArrayStruct(i_cell_item_t) *pendant_elements;
    csmArrayStruct(i_cell_item_t) *all_elements;

    struct i_cell_t *root;
    unsigned long current_visited_timestamp;
};

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_cell_item_t *, i_new_cell_item, (const struct csmoctree_item_t *item, unsigned long last_visited_timestamp))
{
    struct i_cell_item_t *cell_item;
    
    cell_item = MALLOC(struct i_cell_item_t);
    
    cell_item->item = item;
    cell_item->last_visited_timestamp = last_visited_timestamp;
    
    return cell_item;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_cell_item(struct i_cell_item_t **cell_item)
{
    FREE_PP(cell_item, struct i_cell_item_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_cell_t *, i_new_cell, (
                        unsigned long cell_id,
                        struct csmbbox_t **bbox,
                        struct csmbbox_t **extended_bbox,
                        const csmArrayStruct(i_cell_item_t) **edge_elements,
                        const csmArrayStruct(i_cell_item_t) **interior_elements,
                        csmArrayStruct(i_cell_t) **cells))
{
    struct i_cell_t *cell;
    
    cell = MALLOC(struct i_cell_t);
    
    cell->cell_id = cell_id;
    
    cell->bbox = ASSIGN_POINTER_PP_NOT_NULL(bbox, struct csmbbox_t);
    cell->extended_bbox = ASSIGN_POINTER_PP_NOT_NULL(extended_bbox, struct csmbbox_t);
    
    cell->edge_elements = ASSIGN_POINTER_PP_NOT_NULL(edge_elements, const csmArrayStruct(i_cell_item_t));
    cell->interior_elements = ASSIGN_POINTER_PP_NOT_NULL(interior_elements, const csmArrayStruct(i_cell_item_t));
    
    cell->cells = ASSIGN_POINTER_PP(cells, csmArrayStruct(i_cell_t));
    
    return cell;
}

// ----------------------------------------------------------------------------------------------------

static void i_free_cell(struct i_cell_t **cell)
{
    assert_no_null(cell);
    assert_no_null(*cell);
    
    csmbbox_free(&(*cell)->bbox);
    csmbbox_free(&(*cell)->extended_bbox);
    
    csmarrayc_free_const_st(&(*cell)->edge_elements, i_cell_item_t);
    
    if ((*cell)->interior_elements != NULL)
        csmarrayc_free_const_st(&(*cell)->interior_elements, i_cell_item_t);
    
    if ((*cell)->cells != NULL)
        csmarrayc_free_st(&(*cell)->cells, i_cell_t, i_free_cell);
    
    FREE_PP(cell, struct i_cell_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_cell_t *, i_new_root_cell, (struct csmbbox_t **bbox, double tolerance_margin, unsigned long *id_new_element))
{
    unsigned long cell_id;
    struct csmbbox_t *extended_bbox;
    const csmArrayStruct(i_cell_item_t) *edge_elements, *interior_elements;
    csmArrayStruct(i_cell_t) *cells;
    
    assert_no_null(bbox);
    
    cell_id = csmid_new_id(id_new_element, NULL);
    
    extended_bbox = csmbbox_copy(*bbox);
    csmbbox_increase_by_absolute_margin(extended_bbox, tolerance_margin);
    csmbbox_compute_bsphere_and_margins(extended_bbox);
    
    edge_elements = csmarrayc_new_const_st_array(0, i_cell_item_t);
    interior_elements = csmarrayc_new_const_st_array(0, i_cell_item_t);
    
    cells = NULL;
    
    return i_new_cell(cell_id, bbox, &extended_bbox, &edge_elements, &interior_elements, &cells);
}

// ----------------------------------------------------------------------------------------------------

static void i_set_cell(
                        unsigned long cell_idx,
                        double x_min, double y_min, double z_min, double x_max, double y_max, double z_max,
                        double tolerance_margin,
                        unsigned long *id_new_element,
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
    
    cell = i_new_root_cell(&cell_bbox, tolerance_margin, id_new_element);
    csmarrayc_set_st(cells, cell_idx, cell, i_cell_t);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static csmArrayStruct(i_cell_t) *, i_explode_cell_bbox, (const struct csmbbox_t *bbox, double tolerance_margin, unsigned long *id_new_element))
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
    
    i_set_cell(cell_idx++, x_min, y_min, z_min, x_middle, y_middle, z_middle, tolerance_margin, id_new_element, cells);
    i_set_cell(cell_idx++, x_middle, y_min, z_min, x_max, y_middle, z_middle, tolerance_margin, id_new_element, cells);
    i_set_cell(cell_idx++, x_min, y_middle, z_min, x_middle, y_max, z_middle, tolerance_margin, id_new_element, cells);
    i_set_cell(cell_idx++, x_middle, y_middle, z_min, x_max, y_max, z_middle, tolerance_margin, id_new_element, cells);
    
    i_set_cell(cell_idx++, x_min, y_min, z_middle, x_middle, y_middle, z_max, tolerance_margin, id_new_element, cells);
    i_set_cell(cell_idx++, x_middle, y_min, z_middle, x_max, y_middle, z_max, tolerance_margin, id_new_element, cells);
    i_set_cell(cell_idx++, x_min, y_middle, z_middle, x_middle, y_max, z_max, tolerance_margin, id_new_element, cells);
    i_set_cell(cell_idx++, x_middle, y_middle, z_middle, x_max, y_max, z_max, tolerance_margin, id_new_element, cells);
    
    return cells;
}

// ----------------------------------------------------------------------------------------------------

static double i_minimun_cell_size(const struct csmbbox_t *octree_bbox)
{
    double x_min, y_min, z_min, x_max, y_max, z_max;
    double x_extension, y_extension, z_extension;
    
    csmbbox_get_extension_real(octree_bbox, &x_min, &y_min, &z_min, &x_max, &y_max, &z_max);
    
    x_extension = x_max - x_min;
    y_extension = y_max - y_min;
    z_extension = z_max - z_min;
    
    return CSMMATH_MIN(x_extension, CSMMATH_MIN(y_extension, z_extension));
}

// ----------------------------------------------------------------------------------------------------

static CSMBOOL i_needs_to_explode_cell(
                        unsigned long current_occupancy, unsigned long max_occupancy,
                        const struct csmbbox_t *octree_bbox,
                        double minimun_cell_size)
{
    if (current_occupancy < max_occupancy)
    {
        return CSMFALSE;
    }
    else
    {
        double min_octree_bbox_size;
        
        min_octree_bbox_size = i_minimun_cell_size(octree_bbox);
        
        if (min_octree_bbox_size - 1.e-3 < minimun_cell_size)
            return CSMFALSE;
        else
            return CSMTRUE;
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_append_element_to_cell(
                        unsigned long recursion_level,
                        struct i_cell_t *cell,
                        unsigned long max_occupancy,
                        double tolerance_margin,
                        double minimun_cell_size,
                        const struct i_cell_item_t *element,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox,
                        unsigned long *id_new_element,
                        CSMBOOL *added)
{
    CSMBOOL added_loc;
    enum csmoctree_bbox_inters_t res;
    
    assert_no_null(cell);
    assert(recursion_level < 10000);
    assert(max_occupancy > 0);
    assert_no_null(func_intersects_with_bbox);
    assert_no_null(added);
    
    res = func_intersects_with_bbox(element->item, cell->extended_bbox, tolerance_margin);
    
    switch (res)
    {
        case CSMOCTREE_BBOX_INTERS_NO:
            
            added_loc = CSMFALSE;
            break;
            
        case CSMOCTREE_BBOX_INTERS_INTERIOR:
        {
            if (cell->cells == NULL)
            {
                csmarrayc_append_element_st(cell->interior_elements, element, i_cell_item_t);
                added_loc = CSMTRUE;
            }
            else
            {
                unsigned long i, no_cells;
                
                no_cells = csmarrayc_count_st(cell->cells, i_cell_t);
                assert(no_cells == 8);
                
                added_loc = CSMFALSE;
                
                for (i = 0; i < no_cells; i++)
                {
                    struct i_cell_t *inner_cell;
                    
                    inner_cell = csmarrayc_get_st(cell->cells, i, i_cell_t);
                    i_append_element_to_cell(recursion_level + 1, inner_cell, max_occupancy, tolerance_margin, minimun_cell_size, element, func_intersects_with_bbox, id_new_element, &added_loc);
                    
                    if (added_loc == CSMTRUE)
                        break;
                }
                
                assert(added_loc == CSMTRUE);
            }
            break;
        }
            
        case CSMOCTREE_BBOX_INTERS_IN_BBOX_FACE:
        {
            csmarrayc_append_element_st(cell->edge_elements, element, i_cell_item_t);
            added_loc = CSMTRUE;
            break;
        }
            
        default_error();
    }
    
    if (added_loc == CSMTRUE && cell->cells == NULL)
    {
        unsigned long current_occupancy;
        
        current_occupancy = csmarrayc_count_st(cell->interior_elements, i_cell_item_t);

        if (i_needs_to_explode_cell(current_occupancy, max_occupancy, cell->bbox, minimun_cell_size) == CSMTRUE)
        {
            unsigned long i;
            
            cell->cells = i_explode_cell_bbox(cell->bbox, tolerance_margin, id_new_element);
            assert(csmarrayc_count_st(cell->cells, i_cell_t) == 8);
            
            for (i = 0; i < current_occupancy; i++)
            {
                const struct i_cell_item_t *interior_element;
                CSMBOOL added_at_least_in_one_cell;
                unsigned long j;
                
                interior_element = csmarrayc_get_st(cell->interior_elements, i, i_cell_item_t);
                added_at_least_in_one_cell = CSMFALSE;

                for (j = 0; j < 8; j++)
                {
                    struct i_cell_t *inner_cell;
                    CSMBOOL added_i;
                    
                    inner_cell = csmarrayc_get_st(cell->cells, j, i_cell_t);
                    i_append_element_to_cell(recursion_level + 1, inner_cell, max_occupancy, tolerance_margin, minimun_cell_size, interior_element, func_intersects_with_bbox, id_new_element, &added_i);
                    
                    if (added_i == CSMTRUE)
                        added_at_least_in_one_cell = CSMTRUE;
                }
                
                assert(added_at_least_in_one_cell == CSMTRUE);
            }
            
            csmarrayc_free_const_st(&cell->interior_elements, i_cell_item_t);
            cell->interior_elements = csmarrayc_new_const_st_array(0, i_cell_item_t);
        }
    }
    
    *added = added_loc;
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmoctree_t *, i_new, (
                        unsigned long id_new_element,
                        unsigned long max_occupancy,
                        double tolerance_margin,
                        struct csmbbox_t **octree_bbox, double minimun_cell_size,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox,
                        const csmArrayStruct(i_cell_item_t) **pendant_elements,
                        csmArrayStruct(i_cell_item_t) **all_elements,
                        struct i_cell_t **root,
                        unsigned long current_visited_timestamp))
{
    struct csmoctree_t *octree;
    
    octree = MALLOC(struct csmoctree_t);
    
    octree->id_new_element = id_new_element;
    
    octree->max_occupancy = max_occupancy;
    octree->tolerance_margin = tolerance_margin;
    
    octree->octree_bbox = ASSIGN_POINTER_PP_NOT_NULL(octree_bbox, struct csmbbox_t);
    octree->minimun_cell_size = minimun_cell_size;
    
    octree->func_intersects_with_bbox = func_intersects_with_bbox;
    
    octree->pendant_elements = ASSIGN_POINTER_PP_NOT_NULL(pendant_elements, const csmArrayStruct(i_cell_item_t));
    octree->all_elements = ASSIGN_POINTER_PP_NOT_NULL(all_elements, csmArrayStruct(i_cell_item_t));
    
    octree->root = ASSIGN_POINTER_PP(root, struct i_cell_t);
    
    octree->current_visited_timestamp = current_visited_timestamp;
    
    return octree;
}

// ----------------------------------------------------------------------------------------------------

static double i_compute_minimun_cell_size(const struct csmbbox_t *octree_bbox, double tolerance_margin)
{
    double min_cell_size;
    
    assert(tolerance_margin > 0.);
    
    min_cell_size = i_minimun_cell_size(octree_bbox);
    return CSMMATH_MAX(min_cell_size / 10., 4. * tolerance_margin);
}

// ----------------------------------------------------------------------------------------------------

struct csmoctree_t *csmoctree_dontuse_new(
                        unsigned long max_occupancy,
                        double tolerance_margin,
                        struct csmbbox_t **octree_bbox,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox)
{
    unsigned long id_new_element;
    double minimun_cell_size;
    const csmArrayStruct(i_cell_item_t) *pendant_elements;
    csmArrayStruct(i_cell_item_t) *all_elements;
    struct i_cell_t *root;
    unsigned long current_visited_timestamp;
    
    assert_no_null(octree_bbox);
    
    id_new_element = 0;
    
    minimun_cell_size = i_compute_minimun_cell_size(*octree_bbox, tolerance_margin);
    pendant_elements = csmarrayc_new_const_st_array(0, i_cell_item_t);
    all_elements = csmarrayc_new_st_array(0, i_cell_item_t);
    root = NULL;
    current_visited_timestamp = 0;

    return i_new(
                id_new_element,
                max_occupancy,
                tolerance_margin,
                octree_bbox, minimun_cell_size,
                func_intersects_with_bbox,
                &pendant_elements,
                &all_elements,
                &root,
                current_visited_timestamp);
}

// ----------------------------------------------------------------------------------------------------

void csmoctree_dontuse_free(struct csmoctree_t **octree)
{
    assert_no_null(octree);
    assert_no_null(*octree);
    
    csmarrayc_free_const_st(&(*octree)->pendant_elements, i_cell_item_t);
    csmarrayc_free_st(&(*octree)->all_elements, i_cell_item_t, i_free_cell_item);
    
    csmbbox_free(&(*octree)->octree_bbox);
    
    if ((*octree)->root != NULL)
        i_free_cell(&(*octree)->root);
    
    FREE_PP(octree, struct csmoctree_t);
}

// ----------------------------------------------------------------------------------------------------

void csmoctree_dontuse_append_item(struct csmoctree_t *octree, const struct csmoctree_item_t *octree_item)
{
    struct i_cell_item_t *cell_item;
    
    assert_no_null(octree);
    
    cell_item = i_new_cell_item(octree_item, ULONG_MAX);
    csmarrayc_append_element_const_st(octree->pendant_elements, cell_item, i_cell_item_t);
    csmarrayc_append_element_st(octree->all_elements, cell_item, i_cell_item_t);
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
        octree->root = i_new_root_cell(&root_bbox, octree->tolerance_margin, &octree->id_new_element);
    }
    
    no_pendant_elements = csmarrayc_count_st(octree->pendant_elements, i_cell_item_t);
    
    if (no_pendant_elements > 0)
    {
        unsigned long i;
        
        for (i = 0; i < no_pendant_elements; i++)
        {
            const struct i_cell_item_t *element;
            CSMBOOL added;
            
            element = csmarrayc_get_const_st(octree->pendant_elements, i, i_cell_item_t);
            
            i_append_element_to_cell(
                        0,
                        octree->root,
                        octree->max_occupancy, octree->tolerance_margin, octree->minimun_cell_size,
                        element,
                        octree->func_intersects_with_bbox,
                        &octree->id_new_element,
                        &added);
            assert(added == CSMTRUE);
        };
        
        csmarrayc_free_const_st(&octree->pendant_elements, i_cell_item_t);
        octree->pendant_elements = csmarrayc_new_const_st_array(0, i_cell_item_t);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_append_intersected_elements_in_list(
                        const csmArrayStruct(i_cell_item_t) *elements,
                        const struct csmbbox_t *bbox,
                        double tolerance_margin,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox,
                        unsigned long current_visited_timestamp,
                        const csmArrayStruct(csmoctree_item_t) *bbox_neighbors)
{
    unsigned long i, no_elements;

    no_elements = csmarrayc_count_st(elements, i_cell_item_t);

    for (i = 0; i < no_elements; i++)
    {
        struct i_cell_item_t *element;
        
        element = csmarrayc_get_st(elements, i, i_cell_item_t);
        assert_no_null(element);
        
        if (element->last_visited_timestamp != current_visited_timestamp
                && func_intersects_with_bbox(element->item, bbox, tolerance_margin) != CSMOCTREE_BBOX_INTERS_NO)
        {
            element->last_visited_timestamp = current_visited_timestamp;
            csmarrayc_append_element_const_st(bbox_neighbors, element->item, csmoctree_item_t);
        }
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_append_bbox_neighbors(
                        unsigned long recursion_level,
                        const struct i_cell_t *cell,
                        const struct csmbbox_t *bbox,
                        double tolerance_margin,
                        csmoctree_FPtr_intersects_with_bbox func_intersects_with_bbox,
                        unsigned long current_visited_timestamp,
                        const csmArrayStruct(csmoctree_item_t) *bbox_neighbors)
{
    assert_no_null(cell);
    assert(recursion_level < 10000);
    assert_no_null(func_intersects_with_bbox);
    
    if (csmbbox_intersects_with_other_bbox(cell->extended_bbox, bbox) == CSMTRUE)
    {
        i_append_intersected_elements_in_list(cell->edge_elements, bbox, tolerance_margin, func_intersects_with_bbox, current_visited_timestamp, bbox_neighbors);
        
        if (cell->cells == NULL)
        {
            i_append_intersected_elements_in_list(cell->interior_elements, bbox, tolerance_margin, func_intersects_with_bbox, current_visited_timestamp, bbox_neighbors);
        }
        else
        {
            unsigned long i;
            
            assert(csmarrayc_count_st(cell->interior_elements, i_cell_item_t) == 0);
            assert(csmarrayc_count_st(cell->cells, i_cell_t) == 8);
            
            for (i = 0; i < 8; i++)
            {
                struct i_cell_t *inner_cell;
                
                inner_cell = csmarrayc_get_st(cell->cells, i, i_cell_t);
                i_append_bbox_neighbors(recursion_level + 1, inner_cell, bbox, tolerance_margin, func_intersects_with_bbox, current_visited_timestamp, bbox_neighbors);
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
    i_append_bbox_neighbors(0, octree->root, bbox, octree->tolerance_margin, octree->func_intersects_with_bbox, octree->current_visited_timestamp, bbox_neighbors);

    octree->current_visited_timestamp++;
    
    return bbox_neighbors;
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_elements_in_list(
                        const csmArrayStruct(i_cell_item_t) *elements,
                        csmoctree_FPtr_print_debug_info func_print_debug_info)
{
    unsigned long i, no_elements;

    assert_no_null(func_print_debug_info);
    
    no_elements = csmarrayc_count_st(elements, i_cell_item_t);
    csmdebug_print_debug_info("\tCount: %lu\n", no_elements);

    for (i = 0; i < no_elements; i++)
    {
        struct i_cell_item_t *element;
        
        element = csmarrayc_get_st(elements, i, i_cell_item_t);
        assert_no_null(element);
        
        func_print_debug_info(element->item);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_print_cell_recursively(const struct i_cell_t *cell, unsigned long nesting_level, csmoctree_FPtr_print_debug_info func_print_debug_info)
{
    double x_min, y_min, z_min, x_max, y_max, z_max;
    
    assert_no_null(cell);
    assert_no_null(func_print_debug_info);

    if (csmarrayc_count_st(cell->edge_elements, i_cell_item_t) > 0 || csmarrayc_count_st(cell->interior_elements, i_cell_item_t) > 0)
    {
        csmdebug_print_debug_info("Id: %lu, Level: %lu\n", cell->cell_id, nesting_level);

        csmbbox_get_extension_real(cell->bbox, &x_min, &y_min, &z_min, &x_max, &y_max, &z_max);
        csmdebug_print_debug_info("\tBbox: %lf, %lf, %lf -- %lf, %lf, %lf \n", x_min, y_min, z_min, x_max, y_max, z_max);
        
        csmdebug_print_debug_info("edge elements\n");
        i_print_debug_info_elements_in_list(cell->edge_elements, func_print_debug_info);
        
        csmdebug_print_debug_info("interior elements\n");
        i_print_debug_info_elements_in_list(cell->interior_elements, func_print_debug_info);
    }
    
    if (cell->cells != NULL)
    {
        unsigned long i, no_cells;
    
        no_cells = csmarrayc_count_st(cell->cells, i_cell_t);
        assert(no_cells == 8);
    
        for (i = 0; i < no_cells; i++)
        {
            struct i_cell_t *inner_cell;
            
            inner_cell = csmarrayc_get_st(cell->cells, i, i_cell_t);
            i_print_cell_recursively(inner_cell, nesting_level + 1, func_print_debug_info);
        }
    }
}

// ----------------------------------------------------------------------------------------------------

void csmoctree_dontuse_print(struct csmoctree_t *octree, csmoctree_FPtr_print_debug_info func_print_debug_info)
{
    assert_no_null(octree);
    
    i_update_octree(octree);
    i_print_cell_recursively(octree->root, 0, func_print_debug_info);
}
