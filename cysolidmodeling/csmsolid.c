//
//  csmsolid.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 23/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsolid.h"
#include "csmsolid.inl"
#include "csmsolid.tli"

#include "csmbbox.inl"
#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmmaterial.inl"
#include "csmnode.inl"
#include "csmmath.inl"
#include "csmtransform.inl"
#include "csmvertex.inl"
#include "csmassert.inl"
#include "csmid.inl"
#include "csmmem.inl"
#include "csmstring.inl"

struct i_item_t;
struct csmhashtb(i_item_t);

typedef void (*i_FPtr_reassign_id)(struct i_item_t *item, unsigned long *id_nuevo_elemento, unsigned long *new_id_opc);
#define i_CHECK_FUNC_REASSIGN_ID(function, type) ((void(*)(struct type *, unsigned long *, unsigned long *))function == function)

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_crea, (
                        char **name,
                        unsigned long id_nuevo_elemento,
                        struct csmhashtb(csmface_t) **sfaces,
                        struct csmhashtb(csmedge_t) **sedges,
                        struct csmhashtb(csmvertex_t) **svertexs,
                        struct csmmaterial_t **visz_material_opt,
                        CSMBOOL draw_only_border_edges,
                        struct csmbbox_t **bbox))
{
    struct csmsolid_t *solido;
    
    solido = MALLOC(struct csmsolid_t);
    
    solido->name = ASIGNA_PUNTERO_PP(name, char);
    
    solido->id_nuevo_elemento = id_nuevo_elemento;
    
    solido->sfaces = ASIGNA_PUNTERO_PP_NO_NULL(sfaces, struct csmhashtb(csmface_t));
    solido->sedges = ASIGNA_PUNTERO_PP_NO_NULL(sedges, struct csmhashtb(csmedge_t));
    solido->svertexs = ASIGNA_PUNTERO_PP_NO_NULL(svertexs, struct csmhashtb(csmvertex_t));
    
    solido->visz_material_opt = ASIGNA_PUNTERO_PP(visz_material_opt, struct csmmaterial_t);
    solido->draw_only_border_edges = draw_only_border_edges;
    
    solido->bbox = ASIGNA_PUNTERO_PP_NO_NULL(bbox, struct csmbbox_t);
    
    return solido;
}

// ----------------------------------------------------------------------------------------------------

struct csmsolid_t *csmsolid_crea_vacio(unsigned long start_id_of_new_element)
{
    char *name;
    unsigned long id_nuevo_elemento;
    struct csmhashtb(csmface_t) *sfaces;
    struct csmhashtb(csmedge_t) *sedges;
    struct csmhashtb(csmvertex_t) *svertexs;
    struct csmmaterial_t *visz_material_opt;
    CSMBOOL draw_only_border_edges;
    struct csmbbox_t *bbox;
    
    name = NULL;
    
    id_nuevo_elemento = start_id_of_new_element;
    
    sfaces = csmhashtb_create_empty(csmface_t);
    sedges = csmhashtb_create_empty(csmedge_t);
    svertexs = csmhashtb_create_empty(csmvertex_t);
    
    visz_material_opt = NULL;
    draw_only_border_edges = CSMTRUE;
    
    bbox = csmbbox_create_empty_box();
    
    return i_crea(&name, id_nuevo_elemento, &sfaces, &sedges, &svertexs, &visz_material_opt, draw_only_border_edges, &bbox);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_set_name(struct csmsolid_t *solid, const char *name)
{
    assert_no_null(solid);
    
    if (solid->name != NULL)
        csmstring_free(&solid->name);
    
    solid->name = csmstring_duplicate(name);
}

// ----------------------------------------------------------------------------------------------------

const char *csmsolid_get_name(const struct csmsolid_t *solid)
{
    assert_no_null(solid);
    return solid->name;
}

// ----------------------------------------------------------------------------------------------------

const struct csmmaterial_t *csmsolid_get_material(const struct csmsolid_t *solido)
{
    assert_no_null(solido);
    return solido->visz_material_opt;
}

// ----------------------------------------------------------------------------------------------------

const struct csmbbox_t *csmsolid_get_bbox(const struct csmsolid_t *solido)
{
    assert_no_null(solido);
    return solido->bbox;
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_duplicate_solid, (
	                    const char *name,
                        unsigned long id_nuevo_elemento,
                        const struct csmmaterial_t *solid_visz_material_opt,
                        CSMBOOL draw_only_border_edges,
                        const struct csmbbox_t *bbox))
{
    char *name_copy;
    struct csmhashtb(csmface_t) *sfaces;
    struct csmhashtb(csmedge_t) *sedges;
    struct csmhashtb(csmvertex_t) *svertexs;
    struct csmmaterial_t *visz_material_opt;
    struct csmbbox_t *bbox_copy;
    
    if (name != NULL)
        name_copy = csmstring_duplicate(name);
    else
        name_copy = NULL;
    
    sfaces = csmhashtb_create_empty(csmface_t);
    sedges = csmhashtb_create_empty(csmedge_t);
    svertexs = csmhashtb_create_empty(csmvertex_t);
    
    if (solid_visz_material_opt != NULL)
        visz_material_opt = csmmaterial_copy(solid_visz_material_opt);
    else
        visz_material_opt = NULL;
    
    bbox_copy = csmbbox_copy(bbox);
    
    return i_crea(&name_copy, id_nuevo_elemento, &sfaces, &sedges, &svertexs, &visz_material_opt, draw_only_border_edges, &bbox_copy);
}

// ----------------------------------------------------------------------------------------------------

static void i_duplicate_vertexs_table(
                        const struct csmhashtb(csmvertex_t) *svertexs,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *new_svertexs,
                        struct csmhashtb(csmvertex_t) **relation_svertexs_old_to_new)
{
    struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new_loc;
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    
    assert_no_null(relation_svertexs_old_to_new);
    
    relation_svertexs_old_to_new_loc = csmhashtb_create_empty(csmvertex_t);
    
    iterator = csmhashtb_create_iterator(svertexs, csmvertex_t);
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CSMTRUE)
    {
        struct csmvertex_t *old_vertex;
        struct csmvertex_t *new_vertex;
        
        csmhashtb_next_pair(iterator, NULL, &old_vertex, csmvertex_t);
        
        new_vertex = csmvertex_duplicate(old_vertex, id_nuevo_elemento, relation_svertexs_old_to_new_loc);
        csmhashtb_add_item(new_svertexs, csmvertex_id(new_vertex), new_vertex, csmvertex_t);
    }
    
    *relation_svertexs_old_to_new = relation_svertexs_old_to_new_loc;
    
    csmhashtb_free_iterator(&iterator, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_duplicate_faces_table(
                        struct csmhashtb(csmface_t) *sfaces,
                        struct csmsolid_t *fsolid,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmface_t) *new_sfaces,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) **relation_shedges_old_to_new)
{
    struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new_loc;
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    assert_no_null(relation_svertexs_old_to_new);
    
    relation_shedges_old_to_new_loc = csmhashtb_create_empty(csmhedge_t);
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    while (csmhashtb_has_next(iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *old_face;
        struct csmface_t *new_face;
        
        csmhashtb_next_pair(iterator, NULL, &old_face, csmface_t);
        
        new_face = csmface_duplicate(old_face, fsolid, id_nuevo_elemento, relation_svertexs_old_to_new, relation_shedges_old_to_new_loc);
        csmhashtb_add_item(new_sfaces, csmface_id(new_face), new_face, csmface_t);
    }
    
    *relation_shedges_old_to_new = relation_shedges_old_to_new_loc;
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_duplicate_edges_table(
                        struct csmhashtb(csmedge_t) *sedges,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmedge_t) *new_sedges)
{
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CSMTRUE)
    {
        struct csmedge_t *old_edge;
        struct csmedge_t *new_edge;
        
        csmhashtb_next_pair(iterator, NULL, &old_edge, csmedge_t);
        
        new_edge = csmedge_duplicate(old_edge, id_nuevo_elemento, relation_shedges_old_to_new);
        csmhashtb_add_item(new_sedges, csmedge_id(new_edge), new_edge, csmedge_t);
    }
    
    csmhashtb_free_iterator(&iterator, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

struct csmsolid_t *csmsolid_duplicate(const struct csmsolid_t *solid)
{
    struct csmsolid_t *new_solid;
    struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new;
    struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new;
    
    assert_no_null(solid);
    
    new_solid = i_duplicate_solid(
                        solid->name,
                        solid->id_nuevo_elemento,
                        solid->visz_material_opt,
                        solid->draw_only_border_edges,
                        solid->bbox);
    assert_no_null(new_solid);

    i_duplicate_vertexs_table(solid->svertexs, &new_solid->id_nuevo_elemento, new_solid->svertexs, &relation_svertexs_old_to_new);
    i_duplicate_faces_table(solid->sfaces, new_solid, &new_solid->id_nuevo_elemento, new_solid->sfaces, relation_svertexs_old_to_new, &relation_shedges_old_to_new);
    i_duplicate_edges_table(solid->sedges, relation_shedges_old_to_new, &new_solid->id_nuevo_elemento, new_solid->sedges);
    
    csmhashtb_free(&relation_svertexs_old_to_new, csmvertex_t, NULL);
    csmhashtb_free(&relation_shedges_old_to_new, csmhedge_t, NULL);
    
    return new_solid;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_free(struct csmsolid_t **solido)
{
    assert_no_null(solido);
    assert_no_null(*solido);
    
    if ((*solido)->name != NULL)
        csmstring_free(&(*solido)->name);
    
    csmhashtb_free(&(*solido)->sfaces, csmface_t, csmface_destruye);
    csmhashtb_free(&(*solido)->sedges, csmedge_t, csmedge_destruye);
    csmhashtb_free(&(*solido)->svertexs, csmvertex_t, csmvertex_destruye);

    if ((*solido)->visz_material_opt != NULL)
        csmmaterial_free(&(*solido)->visz_material_opt);
    
    FREE_PP(solido, struct csmsolid_t);
}

// ----------------------------------------------------------------------------------------------------

unsigned long *csmsolid_id_new_element(struct csmsolid_t *solido)
{
    assert_no_null(solido);
    return &solido->id_nuevo_elemento;
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmsolid_is_empty(const struct csmsolid_t *solido)
{
    assert_no_null(solido);
    
    if (csmhashtb_count(solido->sedges, csmedge_t) > 0)
        return CSMFALSE;
    
    if (csmhashtb_count(solido->sfaces, csmface_t) > 0)
        return CSMFALSE;
    
    if (csmhashtb_count(solido->svertexs, csmvertex_t) > 0)
        return CSMFALSE;
    
    return CSMTRUE;
}

// ----------------------------------------------------------------------------------------------------

static void i_clear_algorithm_vertex_mask(struct csmhashtb(csmvertex_t) *svertexs)
{
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    
    iterator = csmhashtb_create_iterator(svertexs, csmvertex_t);
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CSMTRUE)
    {
        struct csmvertex_t *vertex;
        
        csmhashtb_next_pair(iterator, NULL, &vertex, csmvertex_t);
        csmvertex_clear_mask(vertex);
    }
    
    csmhashtb_free_iterator(&iterator, csmvertex_t);
}
                                          
// ----------------------------------------------------------------------------------------------------

static void i_clear_algorithm_face_mask(struct csmhashtb(csmface_t) *sfaces)
{
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    while (csmhashtb_has_next(iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        csmface_clear_algorithm_mask(face);
    }
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_clear_algorithm_edge_mask(struct csmhashtb(csmedge_t) *sedges)
{
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CSMTRUE)
    {
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, NULL, &edge, csmedge_t);
        csmedge_setop_set_is_null_edge(edge, CSMFALSE);
    }
    
    csmhashtb_free_iterator(&iterator, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_clear_algorithm_data(struct csmsolid_t *solid)
{
    assert_no_null(solid);
    
    i_clear_algorithm_vertex_mask(solid->svertexs);
    i_clear_algorithm_edge_mask(solid->sedges);
    i_clear_algorithm_face_mask(solid->sfaces);
}

// ----------------------------------------------------------------------------------------------------

static void i_redo_faces_geometric_generated_data(struct csmhashtb(csmface_t) *sfaces, struct csmbbox_t *bbox)
{
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    csmbbox_reset(bbox);
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    while (csmhashtb_has_next(iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        const struct csmbbox_t *face_bbox;
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        
        csmface_redo_geometric_generated_data(face);
        
        face_bbox = csmface_bbox(face);
        csmbbox_maximize_bbox(bbox, face_bbox);
    }
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ------------------------------------------------------------------------------------------

static struct csmhedge_t *i_he_mate(struct csmhedge_t *hedge)
{
    struct csmedge_t *edge;
    
    edge = csmhedge_edge(hedge);
    return csmedge_mate(edge, hedge);
}

// ------------------------------------------------------------------------------------------

static struct csmface_t *i_face_from_hedge(struct csmhedge_t *hedge)
{
    struct csmloop_t *loop;
    
    loop = csmhedge_loop(hedge);
    return csmloop_lface(loop);
}

// ----------------------------------------------------------------------------------------------------

static void i_add_he_face_normal(struct csmhedge_t *he, double *Nx, double *Ny, double *Nz, unsigned long *num_faces)
{
    struct csmface_t *face;
    double A, B, C, D;
    
    assert_no_null(Nx);
    assert_no_null(Ny);
    assert_no_null(Nz);
    assert_no_null(num_faces);
    
    face = i_face_from_hedge(he);
    csmface_face_equation(face, &A, &B, &C, &D);
    
    *Nx += A;
    *Ny += B;
    *Nz += C;
    (*num_faces)++;
}

// ----------------------------------------------------------------------------------------------------

static void i_redo_vertex_normals(struct csmhashtb(csmvertex_t) *svertexs)
{
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    
    iterator = csmhashtb_create_iterator(svertexs, csmvertex_t);
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CSMTRUE)
    {
        struct csmvertex_t *vertex;
        struct csmhedge_t *he_vertex, *he_iterator;
        double Nx, Ny, Nz;
        unsigned long num_faces;
        
        csmhashtb_next_pair(iterator, NULL, &vertex, csmvertex_t);
        
        he_vertex = csmvertex_hedge(vertex);
        he_iterator = he_vertex;
        
        Nx = 0.;
        Ny = 0.;
        Nz = 0.;
        num_faces = 0;
        
        do
        {
            struct csmhedge_t *he_iterator_mate;
            
            i_add_he_face_normal(he_iterator, &Nx, &Ny, &Nz, &num_faces);
            
            he_iterator_mate = i_he_mate(he_iterator);
            i_add_he_face_normal(he_iterator_mate, &Nx, &Ny, &Nz, &num_faces);
            
            he_iterator = csmhedge_next(he_iterator_mate);
            
        } while (he_iterator != he_vertex);
        
        assert(num_faces > 0);
        
        Nx /= num_faces;
        Ny /= num_faces;
        Nz /= num_faces;
        
        csmmath_make_unit_vector3D(&Nx, &Ny, &Nz);
        csmvertex_set_normal(vertex, Nx, Ny, Nz);
    }
    
    csmhashtb_free_iterator(&iterator, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_redo_geometric_face_data(struct csmsolid_t *solid)
{
    assert_no_null(solid);
    i_redo_faces_geometric_generated_data(solid->sfaces, solid->bbox);
}

// ----------------------------------------------------------------------------------------------------

static void i_redo_geometric_generated_data(struct csmhashtb(csmvertex_t) *svertexs, struct csmhashtb(csmface_t) *sfaces, struct csmbbox_t *bbox)
{
    i_redo_faces_geometric_generated_data(sfaces, bbox);
    i_redo_vertex_normals(svertexs);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_redo_geometric_generated_data(struct csmsolid_t *solid)
{
    assert_no_null(solid);
    i_redo_geometric_generated_data(solid->svertexs, solid->sfaces, solid->bbox);
}

// ----------------------------------------------------------------------------------------------------

static void i_nousar_move_elements_between_tables(
                        struct csmhashtb(i_item_t) *table_origin,
                        unsigned long *id_nuevo_elemento,
                        i_FPtr_reassign_id func_reassign_id,
                        struct csmhashtb(i_item_t) *table_destination)
{
    struct csmhashtb_iterator(i_item_t) *iterator;
    
    assert_no_null(func_reassign_id);
    
    iterator = csmhashtb_create_iterator(table_origin, i_item_t);
    
    while (csmhashtb_has_next(iterator, i_item_t) == CSMTRUE)
    {
        unsigned long item_id;
        struct i_item_t *item;
        
        csmhashtb_next_pair(iterator, NULL, &item, i_item_t);
        func_reassign_id(item, id_nuevo_elemento, &item_id);
        
        csmhashtb_add_item(table_destination, item_id, item, i_item_t);
    }
    
    csmhashtb_clear(table_origin, i_item_t, NULL);
    csmhashtb_free_iterator(&iterator, i_item_t);
}

#define i_move_elements_between_tables(\
                        table_origin,\
                        id_nuevo_elemento,\
                        func_reassign_id,\
                        table_destination,\
                        type)\
(\
    ((struct csmhashtb(type) *)table_origin == table_origin),\
    ((struct csmhashtb(type) *)table_destination == table_destination),\
    i_CHECK_FUNC_REASSIGN_ID(func_reassign_id, type),\
    i_nousar_move_elements_between_tables(\
                        (struct csmhashtb(i_item_t) *)table_origin,\
                        id_nuevo_elemento,\
                        (i_FPtr_reassign_id)func_reassign_id,\
                        (struct csmhashtb(i_item_t) *)table_destination)\
)

// ----------------------------------------------------------------------------------------------------

static void i_assign_faces_to_solid(struct csmhashtb(csmface_t) *sfaces, struct csmsolid_t *solid)
{
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    while (csmhashtb_has_next(iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        csmface_set_fsolid(face, solid);
    }
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_merge_solids(struct csmsolid_t *solid, struct csmsolid_t *solid_to_merge)
{
    assert_no_null(solid);
    assert_no_null(solid_to_merge);

    i_assign_faces_to_solid(solid_to_merge->sfaces, solid);
    
    i_move_elements_between_tables(
                        solid_to_merge->sfaces,
                        &solid->id_nuevo_elemento,
                        csmface_reassign_id,
                        solid->sfaces,
                        csmface_t);
    
    i_move_elements_between_tables(
                        solid_to_merge->sedges,
                        &solid->id_nuevo_elemento,
                        csmedge_reassign_id,
                        solid->sedges,
                        csmedge_t);
    
    i_move_elements_between_tables(
                        solid_to_merge->svertexs,
                        &solid->id_nuevo_elemento,
                        csmvertex_reassign_id,
                        solid->svertexs,
                        csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_revert(struct csmsolid_t *solid)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(solid);

    face_iterator = csmhashtb_create_iterator(solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        csmface_revert(face);
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_prepare_for_cleanup(struct csmsolid_t *solid)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(solid);

    face_iterator = csmhashtb_create_iterator(solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        csmface_set_fsolid_aux(face, solid);
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_finish_cleanup(struct csmsolid_t *solid)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(solid);

    face_iterator = csmhashtb_create_iterator(solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        csmface_set_fsolid_aux(face, NULL);
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_append_new_face(struct csmsolid_t *solido, struct csmface_t **face)
{
    struct csmface_t *face_loc;
    
    assert_no_null(solido);
    assert_no_null(face);
    
    face_loc = csmface_crea(solido, &solido->id_nuevo_elemento);
    csmhashtb_add_item(solido->sfaces, csmface_id(face_loc), face_loc, csmface_t);
    
    if (solido->visz_material_opt != NULL)
        csmface_set_visualization_material(face_loc, solido->visz_material_opt);
    
    *face = face_loc;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_remove_face(struct csmsolid_t *solido, struct csmface_t **face)
{
    assert_no_null(solido);
    assert_no_null(face);
    
    csmhashtb_remove_item(solido->sfaces, csmface_id(*face), csmface_t);
    csmface_destruye(face);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_move_face_to_solid(struct csmsolid_t *face_solid, struct csmface_t *face, struct csmsolid_t *destination_solid)
{
    assert_no_null(face_solid);
    assert_no_null(destination_solid);
    
    csmdebug_print_debug_info("csmsolid_move_face_to_solid(): face: %lu, solid: %p to solid: %p\n", csmface_id(face), csmface_fsolid(face), destination_solid);
    
    csmhashtb_remove_item(face_solid->sfaces, csmface_id(face), csmface_t);
    
    csmface_reassign_id(face, &destination_solid->id_nuevo_elemento, NULL);
    csmhashtb_add_item(destination_solid->sfaces, csmface_id(face), face, csmface_t);
    
    csmface_set_fsolid(face, destination_solid);
}

// ----------------------------------------------------------------------------------------------------

struct csmhashtb_iterator(csmface_t) *csmsolid_face_iterator(struct csmsolid_t *solid)
{
    assert_no_null(solid);
    return csmhashtb_create_iterator(solid->sfaces, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

struct csmface_t *csmsolid_get_face(struct csmsolid_t *solid, unsigned long id_face)
{
    assert_no_null(solid);
    return csmhashtb_ptr_for_id(solid->sfaces, id_face, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_append_new_edge(struct csmsolid_t *solido, struct csmedge_t **edge)
{
    struct csmedge_t *edge_loc;
    
    assert_no_null(solido);
    assert_no_null(edge);
    
    edge_loc = csmedge_crea(&solido->id_nuevo_elemento);
    csmhashtb_add_item(solido->sedges, csmedge_id(edge_loc), edge_loc, csmedge_t);
    
    *edge = edge_loc;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_remove_edge(struct csmsolid_t *solido, struct csmedge_t **edge)
{
    assert_no_null(solido);
    assert_no_null(edge);

    csmhashtb_remove_item(solido->sedges, csmedge_id(*edge), csmedge_t);
    csmedge_destruye(edge);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_move_edge_to_solid(struct csmsolid_t *edge_solid, struct csmedge_t *edge, struct csmsolid_t *destination_solid)
{
    assert_no_null(edge_solid);
    assert_no_null(destination_solid);
    assert(edge_solid != destination_solid);
    
    csmhashtb_remove_item(edge_solid->sedges, csmedge_id(edge), csmedge_t);
    
    csmedge_reassign_id(edge, &destination_solid->id_nuevo_elemento, NULL);
    csmhashtb_add_item(destination_solid->sedges, csmedge_id(edge), edge, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmsolid_contains_edge(const struct csmsolid_t *solid, const struct csmedge_t *edge)
{
    assert_no_null(solid);
    return csmhashtb_contains_id(solid->sedges, csmedge_t, csmedge_id(edge), NULL);
}

// ----------------------------------------------------------------------------------------------------

struct csmhashtb_iterator(csmedge_t) *csmsolid_edge_iterator(struct csmsolid_t *solid)
{
    assert_no_null(solid);
    return csmhashtb_create_iterator(solid->sedges, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_append_new_vertex(struct csmsolid_t *solido, double x, double y, double z, struct csmvertex_t **vertex)
{
    struct csmvertex_t *vertex_loc;
    
    assert_no_null(solido);
    assert_no_null(vertex);
    
    vertex_loc = csmvertex_crea(x, y, z, &solido->id_nuevo_elemento);
    csmhashtb_add_item(solido->svertexs, csmvertex_id(vertex_loc), vertex_loc, csmvertex_t);
    
    *vertex = vertex_loc;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_remove_vertex(struct csmsolid_t *solido, struct csmvertex_t **vertex)
{
    assert_no_null(solido);
    assert_no_null(vertex);

    csmhashtb_remove_item(solido->svertexs, csmvertex_id(*vertex), csmvertex_t);
    csmvertex_destruye(vertex);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_move_vertex_to_solid(struct csmsolid_t *vertex_solid, struct csmvertex_t *vertex, struct csmsolid_t *destination_solid)
{
    assert_no_null(vertex_solid);
    assert_no_null(destination_solid);
    assert(vertex_solid != destination_solid);
    
    csmhashtb_remove_item(vertex_solid->svertexs, csmvertex_id(vertex), csmvertex_t);
    
    csmvertex_reassign_id(vertex, &destination_solid->id_nuevo_elemento, NULL);
    csmhashtb_add_item(destination_solid->svertexs, csmvertex_id(vertex), vertex, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmsolid_contains_vertex(const struct csmsolid_t *solid, const struct csmvertex_t *vertex)
{
    assert_no_null(solid);
    return csmhashtb_contains_id(solid->svertexs, csmvertex_t, csmvertex_id(vertex), NULL);
}

// ----------------------------------------------------------------------------------------------------

struct csmhashtb_iterator(csmvertex_t) *csmsolid_vertex_iterator(struct csmsolid_t *solid)
{
    assert_no_null(solid);
    return csmhashtb_create_iterator(solid->svertexs, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_set_draw_only_border_edges(struct csmsolid_t *solido, CSMBOOL draw_only_border_edges)
{
    assert_no_null(solido);
    solido->draw_only_border_edges = draw_only_border_edges;
}

// ----------------------------------------------------------------------------------------------------

CSMBOOL csmsolid_contains_vertex_in_same_coordinates_as_given(
                        struct csmsolid_t *solid,
                        const struct csmvertex_t *vertex,
                        double tolerance,
                        struct csmvertex_t **coincident_vertex)
{
    CSMBOOL contains_vertex;
    double x, y, z;
    struct csmvertex_t *coincident_vertex_loc;
    
    assert_no_null(solid);
    assert_no_null(coincident_vertex);
    
    contains_vertex = CSMFALSE;
    coincident_vertex_loc = NULL;
    
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    
    if (csmbbox_contains_point(solid->bbox, x, y, z) == CSMTRUE)
    {
        struct csmhashtb_iterator(csmvertex_t) *iterator;
        
        iterator = csmhashtb_create_iterator(solid->svertexs, csmvertex_t);
        
        while (csmhashtb_has_next(iterator, csmvertex_t) == CSMTRUE)
        {
            struct csmvertex_t *vertex_i;
            
            csmhashtb_next_pair(iterator, NULL, &vertex_i, csmvertex_t);
            
            if (csmvertex_equal_coords(vertex, vertex_i, tolerance) == CSMTRUE)
            {
                contains_vertex = CSMTRUE;
                coincident_vertex_loc = vertex_i;
                break;
            }
        }
        
        csmhashtb_free_iterator(&iterator, csmvertex_t);
    }
    
    *coincident_vertex = coincident_vertex_loc;
    
    return contains_vertex;
}

// ----------------------------------------------------------------------------------------------------

static void i_apply_transformation_to_vertexs(
                        struct csmhashtb(csmface_t) *sfaces,
                        struct csmhashtb(csmvertex_t) *svertexs,
                        const struct csmtransform_t *transform,
                        struct csmbbox_t *bbox)
{
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    
    iterator = csmhashtb_create_iterator(svertexs, csmvertex_t);
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CSMTRUE)
    {
        struct csmvertex_t *vertex;
        
        csmhashtb_next_pair(iterator, NULL, &vertex, csmvertex_t);
        csmvertex_apply_transform(vertex, transform);
    }
    
    csmhashtb_free_iterator(&iterator, csmvertex_t);
    
    i_redo_geometric_generated_data(svertexs, sfaces, bbox);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_move(struct csmsolid_t *solid, double dx, double dy, double dz)
{
    struct csmtransform_t *transform;
    
    assert_no_null(solid);
    
    transform = csmtransform_make_displacement(dx, dy, dz);
    i_apply_transformation_to_vertexs(solid->sfaces, solid->svertexs, transform, solid->bbox);
    
    csmtransform_free(&transform);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_rotate(
                    struct csmsolid_t *solid,
                    double angulo_rotacion_rad,
                    double Xo, double Yo, double Zo, double Ux, double Uy, double Uz)
{
    struct csmtransform_t *transform;
    
    assert_no_null(solid);
    
    transform = csmtransform_make_arbitrary_axis_rotation(angulo_rotacion_rad, Xo, Yo, Zo, Ux, Uy, Uz);
    i_apply_transformation_to_vertexs(solid->sfaces, solid->svertexs, transform, solid->bbox);
    
    csmtransform_free(&transform);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_general_transform(
                    struct csmsolid_t *solid,
                    double Ux, double Uy, double Uz, double Dx,
                    double Vx, double Vy, double Vz, double Dy,
                    double Wx, double Wy, double Wz, double Dz)
{
    struct csmtransform_t *transform;
    
    assert_no_null(solid);
    
    transform = csmtransform_make_general(
                    Ux, Uy, Uz, Dx,
                    Vx, Vy, Vz, Dy,
                    Wx, Wy, Wz, Dz);
    
    i_apply_transformation_to_vertexs(solid->sfaces, solid->svertexs, transform, solid->bbox);
    
    csmtransform_free(&transform);
}

// ----------------------------------------------------------------------------------------------------
// Note.: Improve using a point for the signed tetrahedra interior to the solid (or as close as possible but out of it).

double csmsolid_volume(const struct csmsolid_t *solid)
{
    double volume;
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(solid);

    i_redo_geometric_generated_data(solid->svertexs, solid->sfaces, solid->bbox);
    
    volume = 0.;
    face_iterator = csmhashtb_create_iterator(solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        register struct csmloop_t *loop_iterator;
        unsigned long num_loop_iterations;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        loop_iterator = csmface_floops(face);
        num_loop_iterations = 0;
        
        while (loop_iterator != NULL)
        {
            struct csmhedge_t *he1, *he2;
            double x1, y1, z1;
            unsigned long num_he_iterations;
            
            assert(num_loop_iterations < 1000);
            num_loop_iterations++;
            
            he1 = csmloop_ledge(loop_iterator);
            csmvertex_get_coordenadas(csmhedge_vertex(he1), &x1, &y1, &z1);
            
            he2 = csmhedge_next(he1);
            num_he_iterations = 0;
            
            do
            {
                double x2, y2, z2;
                struct csmhedge_t *he2_next;
                double x_next, y_next, z_next;
                double Ux_cross, Uy_cross, Uz_cross;

                csmvertex_get_coordenadas(csmhedge_vertex(he2), &x2, &y2, &z2);
                csmmath_cross_product3D(x2, y2, z2, x1, y1, z1, &Ux_cross, &Uy_cross, &Uz_cross);

                he2_next = csmhedge_next(he2);
                csmvertex_get_coordenadas(csmhedge_vertex(he2_next), &x_next, &y_next, &z_next);
                
                volume += csmmath_dot_product3D(x_next, y_next, z_next, Ux_cross, Uy_cross, Uz_cross);
                
                he2 = he2_next;
            }
            while (he2 != he1);
            
            loop_iterator = csmloop_next(loop_iterator);
        }
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
    
    return volume / 6.;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_set_visualization_material(struct csmsolid_t *solid, struct csmmaterial_t **visz_material)
{
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(solid);
    
    if (solid->visz_material_opt != NULL)
        csmmaterial_free(&solid->visz_material_opt);
    
    solid->visz_material_opt = ASIGNA_PUNTERO_PP_NO_NULL(visz_material, struct csmmaterial_t);
    
    face_iterator = csmhashtb_create_iterator(solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
        csmface_set_visualization_material(face, solid->visz_material_opt);
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
}


















