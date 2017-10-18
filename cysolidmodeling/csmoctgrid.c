// Spatial occupancy datastructure with directory information...

#include "csmoctgrid.inl"

#include "csmarrayc.inl"
#include "csmassert.inl"
#include "csmbbox.inl"
#include "csmhashtb.inl"
#include "csmedge.inl"
#include "csmface.inl"
#include "csmid.inl"
#include "csmmem.inl"
#include "csmsolid.inl"
#include "csmvertex.inl"

#define i_NUM_DIVISIONS 8

struct i_cell_t
{
    unsigned long cell_id;
    
    struct csmbbox_t *bbox;
    
    const csmArrayStruct(csmvertex_t) *vertexs;
    const csmArrayStruct(csmedge_t) *edges;
    const csmArrayStruct(csmface_t) *faces;
};

struct i_element_occupancy_t
{
    unsigned long id;  // Every element has an occupancy id to be retrieved for querys
    csmArrayStruct(i_cell_t) *cells; // Cells occupied by the element
};

struct csmoctgrid_t
{
    struct csmhashtb(i_element_occupancy_t) *elements_occupancy;
    csmArrayStruct(i_cell_t) *cells;
};

enum i_octree_division_t
{
    i_OCTREE_DIVISION_11,
    i_OCTREE_DIVISION_12,
    i_OCTREE_DIVISION_13,
    i_OCTREE_DIVISION_14,
    i_OCTREE_DIVISION_21,
    i_OCTREE_DIVISION_22,
    i_OCTREE_DIVISION_23,
    i_OCTREE_DIVISION_24
};

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_cell_t *, i_new_empty_cell, (
                        unsigned long cell_id,
                        struct csmbbox_t **bbox,
                        const csmArrayStruct(csmvertex_t) **vertexs))
{
    struct i_cell_t *cell;
    
    cell = MALLOC(struct i_cell_t);
    
    cell->cell_id = cell_id;
    
    cell->bbox = ASIGNA_PUNTERO_PP_NO_NULL(bbox, struct csmbbox_t);
    
    cell->vertexs = ASIGNA_PUNTERO_PP_NO_NULL(vertexs, const csmArrayStruct(csmvertex_t));
    
    cell->edges = csmarrayc_new_const_st_array(0, csmedge_t);
    cell->faces = csmarrayc_new_const_st_array(0, csmface_t);
    
    return cell;
}

// --------------------------------------------------------------------------------

static void i_free_cell(struct i_cell_t **cell)
{
    assert_no_null(cell);
    assert_no_null(*cell);
    
    csmbbox_free(&(*cell)->bbox);
    
    csmarrayc_free_const_st(&(*cell)->vertexs, csmvertex_t);
    csmarrayc_free_const_st(&(*cell)->edges, csmedge_t);
    csmarrayc_free_const_st(&(*cell)->faces, csmface_t);
    
    FREE_PP(cell, struct i_cell_t);
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_element_occupancy_t *, i_new_element_occupancy, (
                        unsigned long id,
                        csmArrayStruct(i_cell_t) **cells))
{
    struct i_element_occupancy_t *element_occupancy;
    
    element_occupancy = MALLOC(struct i_element_occupancy_t);
    
    element_occupancy->id = id;
    element_occupancy->cells = ASIGNA_PUNTERO_PP_NO_NULL(cells, csmArrayStruct(i_cell_t));
    
    return element_occupancy;
}

// --------------------------------------------------------------------------------

static void i_free_element_occupancy(struct i_element_occupancy_t **element_occupancy)
{
    assert_no_null(element_occupancy);
    assert_no_null(*element_occupancy);
    
    csmarrayc_free_st(&(*element_occupancy)->cells, i_cell_t, NULL);
    
    FREE_PP(element_occupancy, struct i_element_occupancy_t);
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmoctgrid_t *, i_new_csmoctgrid, (
                    struct csmhashtb(i_element_occupancy_t) **elements_occupancy,
                    csmArrayStruct(i_cell_t) **cells))
{
    struct csmoctgrid_t *octgrid;
    
    octgrid = MALLOC(struct csmoctgrid_t);
    
    octgrid->elements_occupancy = ASIGNA_PUNTERO_PP_NO_NULL(elements_occupancy, struct csmhashtb(i_element_occupancy_t));
    octgrid->cells = ASIGNA_PUNTERO_PP_NO_NULL(cells, csmArrayStruct(i_cell_t));
    
    return octgrid;
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmbbox_t *, i_compute_octree_extension, (const struct csmsolid_t *solid_a, const struct csmsolid_t *solid_b))
{
    struct csmbbox_t *octree_bbox;
    const struct csmbbox_t *bbox_solid_a, *bbox_solid_b;
    
    octree_bbox = csmbbox_create_empty_box();
    
    bbox_solid_a = csmsolid_get_bbox(solid_a);
    csmbbox_maximize_bbox(octree_bbox, bbox_solid_a);
    
    bbox_solid_b = csmsolid_get_bbox(solid_b);
    csmbbox_maximize_bbox(octree_bbox, bbox_solid_b);
    csmbbox_add_margin(octree_bbox);

    return octree_bbox;
}

// --------------------------------------------------------------------------------

static void i_append_solid_vertexs(struct csmsolid_t *solid, const csmArrayStruct(csmvertex_t) *all_vertexs)
{
    struct csmhashtb_iterator(csmvertex_t) *it;
    
    it = csmsolid_vertex_iterator(solid);
    
    while (csmhashtb_has_next(it, csmvertex_t) == CSMTRUE)
    {
        struct csmvertex_t *vertex;
        
        csmhashtb_next_pair(it, NULL, &vertex, csmvertex_t);
        csmarrayc_append_element_const_st(all_vertexs, vertex, csmvertex_t);
    }
    
    csmhashtb_free_iterator(&it, csmvertex_t);
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static const csmArrayStruct(csmvertex_t) *, i_all_vertices, (struct csmsolid_t *solid_a, struct csmsolid_t *solid_b))
{
    const csmArrayStruct(csmvertex_t) *all_vertexs;
    
    all_vertexs = csmarrayc_new_const_st_array(0, csmvertex_t);
    i_append_solid_vertexs(solid_a, all_vertexs);
    i_append_solid_vertexs(solid_b, all_vertexs);
    
    return all_vertexs;
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmbbox_t *, i_get_node_subdivision_extension, (
                        const struct csmbbox_t *bbox,
                        enum i_octree_division_t division))
{
    struct csmbbox_t *subdivision_bbox;
    double x_min, y_min, z_min, x_max, y_max, z_max;
    double x_center, y_center, z_center;
    double x_min_div, y_min_div, z_min_div, x_max_div, y_max_div, z_max_div;
    
    csmbbox_get_extension(bbox, &x_min, &y_min, &z_min, &x_max, &y_max, &z_max);
    
    x_center = .5 * (x_max + x_min);
    y_center = .5 * (y_max + y_min);
    z_center = .5 * (z_max + z_min);
    
    switch (division)
    {
        case i_OCTREE_DIVISION_11:
        {
            x_min_div = x_min;
            y_min_div = y_min;
            
            x_max_div = x_center;
            y_max_div = y_center;
            
            z_min_div = z_min;
            z_max_div = z_center;
            break;
        }
            
        case i_OCTREE_DIVISION_12:
        {
            x_min_div = x_center;
            y_min_div = y_min;
            
            x_max_div = x_max;
            y_max_div = y_center;
            
            z_min_div = z_min;
            z_max_div = z_center;
            break;
        }
            
        case i_OCTREE_DIVISION_13:
        {
            x_min_div = x_center;
            y_min_div = y_center;
            
            x_max_div = x_max;
            y_max_div = y_max;
            
            z_min_div = z_min;
            z_max_div = z_center;
            break;
        }
            
        case i_OCTREE_DIVISION_14:
        {
            x_min_div = x_min;
            y_min_div = y_center;
            
            x_max_div = x_center;
            y_max_div = y_max;
            
            z_min_div = z_min;
            z_max_div = z_center;
            break;
        }
            
        case i_OCTREE_DIVISION_21:
        {
            x_min_div = x_min;
            y_min_div = y_min;
            
            x_max_div = x_center;
            y_max_div = y_center;
            
            z_min_div = z_center;
            z_max_div = z_max;
            break;
        }
            
        case i_OCTREE_DIVISION_22:
        {
            x_min_div = x_center;
            y_min_div = y_min;
            
            x_max_div = x_max;
            y_max_div = y_center;
            
            z_min_div = z_center;
            z_max_div = z_max;
            break;
        }
            
        case i_OCTREE_DIVISION_23:
        {
            x_min_div = x_center;
            y_min_div = y_center;
            
            x_max_div = x_max;
            y_max_div = y_max;
            
            z_min_div = z_center;
            z_max_div = z_max;
            break;
        }
            
        case i_OCTREE_DIVISION_24:
        {
            x_min_div = x_min;
            y_min_div = y_center;
            
            x_max_div = x_center;
            y_max_div = y_max;
            
            z_min_div = z_center;
            z_max_div = z_max;
            break;
        }
            
        default_error();
    }
    
    subdivision_bbox = csmbbox_create_empty_box();
    csmbbox_maximize_coord(subdivision_bbox, x_min_div, y_min_div, z_min_div);
    csmbbox_maximize_coord(subdivision_bbox, x_max_div, y_max_div, z_max_div);
    
    return subdivision_bbox;
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static const csmArrayStruct(csmvertex_t) *, i_compute_vertexs_in_bbox, (
                        const struct csmbbox_t *bbox,
                        const csmArrayStruct(csmvertex_t) *all_vertexs))
{
    const csmArrayStruct(csmvertex_t) *vertexs_in_bbox;
    unsigned long i, num_vertexs;
    
    vertexs_in_bbox = csmarrayc_new_const_st_array(0, csmvertex_t);
    
    num_vertexs = csmarrayc_count_st(all_vertexs, csmvertex_t);
    assert(num_vertexs > 0);
    
    for (i = 0; i < num_vertexs; i++)
    {
        const struct csmvertex_t *vertex;
        double x, y, z;
        
        vertex = csmarrayc_get_const_st(all_vertexs, i, csmvertex_t);
        csmvertex_get_coordenadas(vertex, &x, &y, &z);
        
        if (csmbbox_contains_point(bbox, x, y, z) == CSMTRUE)
            csmarrayc_append_element_const_st(vertexs_in_bbox, vertex, csmvertex_t);
    }
    
    return vertexs_in_bbox;
}

// --------------------------------------------------------------------------------

static void i_build_octree_and_occypancy_table(
                        unsigned long recursion_level, double minimun_node_side_length_prev,
                        struct csmbbox_t **bbox,
                        const csmArrayStruct(csmvertex_t) **vertexs,
                        double minimun_division_size,
                        unsigned long max_no_vertex_in_cell,
                        unsigned long *id_new_occupancy,
                        struct csmhashtb(i_element_occupancy_t) *elements_occupancy,
	                    csmArrayStruct(i_cell_t) *cells)
{
    double minimun_node_side_length;
    unsigned long num_vertexs_in_node;
    
    assert_no_null(bbox);
    assert_no_null(vertexs);
    assert(minimun_division_size > 0.);
    assert(max_no_vertex_in_cell > 0);
    
    minimun_node_side_length = csmbbox_minimun_side_length(*bbox);
    num_vertexs_in_node = csmarrayc_count_st(*vertexs, csmvertex_t);
    
    if (recursion_level > 0)
        assert(minimun_node_side_length_prev > minimun_node_side_length);
    
    if (num_vertexs_in_node > max_no_vertex_in_cell && minimun_node_side_length > minimun_division_size)
    {
        unsigned long i;
        
        for (i = 0; i < i_NUM_DIVISIONS; i++)
        {
            enum i_octree_division_t division;
            struct csmbbox_t *subdivision_bbox;
            const csmArrayStruct(csmvertex_t) *vertexs_in_subdivision;
            
            division = (enum i_octree_division_t)i;
            
            subdivision_bbox = i_get_node_subdivision_extension(*bbox, division);
            vertexs_in_subdivision = i_compute_vertexs_in_bbox(subdivision_bbox, *vertexs);
         
	        i_build_octree_and_occypancy_table(
                        recursion_level + 1, minimun_node_side_length,
                        &subdivision_bbox,
                        &vertexs_in_subdivision,
                        minimun_division_size,
                        max_no_vertex_in_cell,
                        id_new_occupancy,
                        elements_occupancy,
                        cells);
        }
        
        csmbbox_free(bbox);
        csmarrayc_free_const_st(vertexs, csmvertex_t);
    }
    else
    {
        unsigned long cell_id;
        struct i_cell_t *cell;
        
        cell_id = csmid_new_id(id_new_occupancy, NULL);
        
        cell = i_new_empty_cell(cell_id, bbox, vertexs);
        csmarrayc_append_element_st(cells, cell, i_cell_t);
    }
}

// --------------------------------------------------------------------------------

static void i_append_solid_edges_to_cells(struct csmsolid_t *solid, csmArrayStruct(i_cell_t) *cells)
{
    unsigned long num_cells;
    struct csmhashtb_iterator(csmedge_t) *it;
    
    num_cells = csmarrayc_count_st(cells, i_cell_t);
    assert(num_cells > 0);
    
    it = csmsolid_edge_iterator(solid);
    
    while (csmhashtb_has_next(it, csmedge_t) == CSMTRUE)
    {
        struct csmedge_t *edge;
        double x1, y1, z1, x2, y2, z2;
        unsigned long i;
        
        csmhashtb_next_pair(it, NULL, &edge, csmedge_t);
        csmedge_vertex_coordinates(edge, &x1, &y1, &z1, &x2, &y2, &z2);
        
        for (i = 0; i < num_cells; i++)
        {
            struct i_cell_t *cell;
            
            cell = csmarrayc_get_st(cells, i, i_cell_t);
            assert_no_null(cell);
            
            if (csmbbox_intersects_with_segment(cell->bbox, x1, y1, z1, x2, y2, z2) == CSMTRUE)
                csmarrayc_append_element_st(cell->edges, edge, csmedge_t);
        }
    }
    
    csmhashtb_free_iterator(&it, csmedge_t);
}

// --------------------------------------------------------------------------------

static void i_append_solid_faces_to_cells(struct csmsolid_t *solid, csmArrayStruct(i_cell_t) *cells)
{
    unsigned long num_cells;
    struct csmhashtb_iterator(csmface_t) *it;
    
    num_cells = csmarrayc_count_st(cells, i_cell_t);
    assert(num_cells > 0);
    
    it = csmsolid_face_iterator(solid);
    
    while (csmhashtb_has_next(it, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        const struct csmbbox_t *face_bbox;
        unsigned long i;
        
        csmhashtb_next_pair(it, NULL, &face, csmface_t);
        face_bbox = csmface_bbox(face);
        
        for (i = 0; i < num_cells; i++)
        {
            struct i_cell_t *cell;
            
            cell = csmarrayc_get_st(cells, i, i_cell_t);
            assert_no_null(cell);
            
            if (csmbbox_intersects_with_other_bbox(cell->bbox, face_bbox) == CSMTRUE)
                csmarrayc_append_element_st(cell->faces, face, csmface_t);
        }
    }
    
    csmhashtb_free_iterator(&it, csmface_t);
}

// --------------------------------------------------------------------------------

struct csmoctgrid_t *csmoctgrid_build(struct csmsolid_t *solid_a, struct csmsolid_t *solid_b)
{
    struct csmbbox_t *octgrid_bbox;
    const csmArrayStruct(csmvertex_t) *all_vertexs;
    struct csmhashtb(i_element_occupancy_t) *elements_occupancy;
    csmArrayStruct(i_cell_t) *cells;
    double minimun_division_size;
    unsigned long max_no_vertex_in_cell;
    unsigned long id_new_occupancy;
    
    octgrid_bbox = i_compute_octree_extension(solid_a, solid_b);
    all_vertexs = i_all_vertices(solid_a, solid_b);

    minimun_division_size = 0.1 * csmbbox_minimun_side_length(octgrid_bbox);
    max_no_vertex_in_cell = 10;
    
    id_new_occupancy = 0;
    cells = csmarrayc_new_st_array(0, i_cell_t);
    elements_occupancy = csmhashtb_create_empty(i_element_occupancy_t);
    
    i_build_octree_and_occypancy_table(
                        0, 0,
                        &octgrid_bbox,
                        &all_vertexs,
                        minimun_division_size,
                        max_no_vertex_in_cell,
                        &id_new_occupancy,
                        elements_occupancy,
                        cells);
    
    i_append_solid_edges_to_cells(solid_a, cells);
    i_append_solid_faces_to_cells(solid_a, cells);

    i_append_solid_edges_to_cells(solid_b, cells);
    i_append_solid_faces_to_cells(solid_b, cells);
    
    return i_new_csmoctgrid(&elements_occupancy, &cells);
}

// --------------------------------------------------------------------------------

void csmoctgrid_free(struct csmoctgrid_t **octgrid)
{
    assert_no_null(octgrid);
    assert_no_null(*octgrid);
    
    csmhashtb_free(&(*octgrid)->elements_occupancy, i_element_occupancy_t, i_free_element_occupancy);
    csmarrayc_free_st(&(*octgrid)->cells, i_cell_t, i_free_cell);
    
    FREE_PP(octgrid, struct csmoctgrid_t);
}







