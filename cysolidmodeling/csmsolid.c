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

#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmnode.inl"
#include "csmmath.inl"
#include "csmtransform.inl"
#include "csmvertex.inl"

#include "copiafor.h"
#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"
#include "cypestr.h"

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
                        struct csmhashtb(csmvertex_t) **svertexs))
{
    struct csmsolid_t *solido;
    
    solido = MALLOC(struct csmsolid_t);
    
    solido->name = ASIGNA_PUNTERO_PP(name, char);
    
    solido->id_nuevo_elemento = id_nuevo_elemento;
    
    solido->sfaces = ASIGNA_PUNTERO_PP_NO_NULL(sfaces, struct csmhashtb(csmface_t));
    solido->sedges = ASIGNA_PUNTERO_PP_NO_NULL(sedges, struct csmhashtb(csmedge_t));
    solido->svertexs = ASIGNA_PUNTERO_PP_NO_NULL(svertexs, struct csmhashtb(csmvertex_t));
    
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
    
    name = NULL;
    
    id_nuevo_elemento = start_id_of_new_element;
    
    sfaces = csmhashtb_create_empty(csmface_t);
    sedges = csmhashtb_create_empty(csmedge_t);
    svertexs = csmhashtb_create_empty(csmvertex_t);
    
    return i_crea(&name, id_nuevo_elemento, &sfaces, &sedges, &svertexs);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_set_name(struct csmsolid_t *solid, const char *name)
{
    assert_no_null(solid);
    
    if (solid->name != NULL)
        cypestr_destruye(&solid->name);
    
    solid->name = cad_copia_cadena(name);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_duplicate_solid, (const char *name, unsigned long id_nuevo_elemento))
{
    char *name_copy;
    struct csmhashtb(csmface_t) *sfaces;
    struct csmhashtb(csmedge_t) *sedges;
    struct csmhashtb(csmvertex_t) *svertexs;
    
    if (name != NULL)
        name_copy = cad_copia_cadena(name);
    else
        name_copy = NULL;
    
    sfaces = csmhashtb_create_empty(csmface_t);
    sedges = csmhashtb_create_empty(csmedge_t);
    svertexs = csmhashtb_create_empty(csmvertex_t);
    
    return i_crea(&name_copy, id_nuevo_elemento, &sfaces, &sedges, &svertexs);
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
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CIERTO)
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
    
    while (csmhashtb_has_next(iterator, csmface_t) == CIERTO)
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
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CIERTO)
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
    
    new_solid = i_duplicate_solid(solid->name, solid->id_nuevo_elemento);
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
        cypestr_destruye(&(*solido)->name);
    
    csmhashtb_free(&(*solido)->sfaces, csmface_t, csmface_destruye);
    csmhashtb_free(&(*solido)->sedges, csmedge_t, csmedge_destruye);
    csmhashtb_free(&(*solido)->svertexs, csmvertex_t, csmvertex_destruye);
    
    FREE_PP(solido, struct csmsolid_t);
}

// ----------------------------------------------------------------------------------------------------

unsigned long *csmsolid_id_new_element(struct csmsolid_t *solido)
{
    assert_no_null(solido);
    return &solido->id_nuevo_elemento;
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmsolid_is_empty(const struct csmsolid_t *solido)
{
    assert_no_null(solido);
    
    if (csmhashtb_count(solido->sedges, csmedge_t) > 0)
        return FALSO;
    
    if (csmhashtb_count(solido->sfaces, csmface_t) > 0)
        return FALSO;
    
    if (csmhashtb_count(solido->svertexs, csmvertex_t) > 0)
        return FALSO;
    
    return CIERTO;
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_clear_algorithm_vertex_mask(struct csmsolid_t *solid)
{
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    
    assert_no_null(solid);
    
    iterator = csmhashtb_create_iterator(solid->svertexs, csmvertex_t);
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CIERTO)
    {
        struct csmvertex_t *vertex;
        
        csmhashtb_next_pair(iterator, NULL, &vertex, csmvertex_t);
        csmvertex_clear_mask(vertex);
    }
    
    csmhashtb_free_iterator(&iterator, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_redo_geometric_generated_data(struct csmhashtb(csmface_t) *sfaces)
{
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    while (csmhashtb_has_next(iterator, csmface_t) == CIERTO)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        csmface_redo_geometric_generated_data(face);
    }
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_redo_geometric_generated_data(struct csmsolid_t *solid)
{
    assert_no_null(solid);
    i_redo_geometric_generated_data(solid->sfaces);
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
    
    while (csmhashtb_has_next(iterator, i_item_t) == CIERTO)
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
    
    while (csmhashtb_has_next(iterator, csmface_t) == CIERTO)
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
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CIERTO)
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
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CIERTO)
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
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CIERTO)
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

CYBOOL csmsolid_contains_edge(const struct csmsolid_t *solid, const struct csmedge_t *edge)
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

CYBOOL csmsolid_contains_vertex(const struct csmsolid_t *solid, const struct csmvertex_t *vertex)
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

CYBOOL csmsolid_contains_vertex_in_same_coordinates_as_given(
                        struct csmsolid_t *solid,
                        const struct csmvertex_t *vertex,
                        double tolerance,
                        struct csmvertex_t **coincident_vertex)
{
    CYBOOL contains_vertex;
    struct csmvertex_t *coincident_vertex_loc;
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    
    assert_no_null(solid);
    assert_no_null(coincident_vertex);

    contains_vertex = FALSO;
    coincident_vertex_loc = NULL;
    
    iterator = csmhashtb_create_iterator(solid->svertexs, csmvertex_t);
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CIERTO)
    {
        struct csmvertex_t *vertex_i;
        
        csmhashtb_next_pair(iterator, NULL, &vertex_i, csmvertex_t);
        
        if (csmvertex_equal_coords(vertex, vertex_i, tolerance) == CIERTO)
        {
            contains_vertex = CIERTO;
            coincident_vertex_loc = vertex_i;
            break;
        }
    }
    
    *coincident_vertex = coincident_vertex_loc;
    
    csmhashtb_free_iterator(&iterator, csmvertex_t);
    
    return contains_vertex;
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_edges(struct csmhashtb(csmedge_t) *sedges, CYBOOL assert_si_no_es_integro, unsigned long *num_edges)
{
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    assert_no_null(num_edges);
    
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    *num_edges = 0;
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CIERTO)
    {
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, NULL, &edge, csmedge_t);
        
        csmedge_print_debug_info(edge, assert_si_no_es_integro);
        (*num_edges)++;
    }
    
    csmdebug_print_debug_info("\tNo. of edges: %lu\n", *num_edges);
    
    assert(*num_edges == csmhashtb_count(sedges, csmedge_t));
    csmhashtb_free_iterator(&iterator, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_vertex(struct csmvertex_t *vertex, CYBOOL assert_si_no_es_integro)
{
    struct csmnode_t *vertex_node;
    double x, y, z;
    struct csmhedge_t *hedge;
    
    assert_no_null(vertex);
    
    vertex_node = CSMNODE(vertex);
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    
    csmdebug_print_debug_info("\tVertex %6lu\t%6.3lf\t%6.3lf\t%6.3lf ", csmnode_id(vertex_node), x, y, z);
    
    hedge = csmvertex_hedge(vertex);
    
    if (hedge != NULL)
    {
        csmdebug_print_debug_info("He %6lu\n", csmnode_id(CSMNODE(hedge)));
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_vertex(hedge) == vertex);
    }
    else
    {
        csmdebug_print_debug_info("He (null)\n");
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_vertexs(struct csmhashtb(csmvertex_t) *svertexs, CYBOOL assert_si_no_es_integro, unsigned long *num_vertexs)
{
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    unsigned long num_iters;
    
    assert_no_null(num_vertexs);
    
    iterator = csmhashtb_create_iterator(svertexs, csmvertex_t);
    *num_vertexs = 0;
    
    num_iters = 0;
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CIERTO)
    {
        struct csmvertex_t *vertex;
        
        assert(num_iters < 1000000);
        num_iters++;
        
        csmhashtb_next_pair(iterator, NULL, &vertex, csmvertex_t);
        i_print_debug_info_vertex(vertex, assert_si_no_es_integro);
        (*num_vertexs)++;
    }
    
    csmdebug_print_debug_info("\tNo. of vertex: %lu\n", *num_vertexs);
    
    assert(*num_vertexs == csmhashtb_count(svertexs, csmvertex_t));
    csmhashtb_free_iterator(&iterator, csmvertex_t);
}


// ----------------------------------------------------------------------------------------------------

static void i_print_info_debug_faces(
                        struct csmhashtb(csmface_t) *sfaces,
                        struct csmsolid_t *solid,
                        CYBOOL assert_si_no_es_integro,
                        unsigned long *num_faces, unsigned long *num_holes)
{
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    assert_no_null(num_faces);
    assert_no_null(num_holes);
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    *num_faces = 0;
    *num_holes = 0;
    
    while (csmhashtb_has_next(iterator, csmface_t) == CIERTO)
    {
        struct csmface_t *face;
        unsigned long num_holes_loc;
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        
        csmface_print_info_debug(face, assert_si_no_es_integro, &num_holes_loc);
        (*num_faces)++;
        *num_holes += num_holes_loc;
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmface_fsolid(face) == solid);
    }
    
    csmdebug_print_debug_info("\tNo. of faces: %lu\n", *num_faces);
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_print_debug(struct csmsolid_t *solido, CYBOOL assert_si_no_es_integro)
{
    if (csmdebug_debug_enabled() == CIERTO)
    {
        unsigned long num_faces, num_vertexs, num_edges, num_holes;
        
        assert_no_null(solido);

        csmdebug_begin_context("SOLID DESCRIPTION");
        
        if (solido->name != NULL)
            csmdebug_print_debug_info("Solid Address: %s (%p)\n", solido->name, solido);
        else
            csmdebug_print_debug_info("Solid Address: %p\n", solido);
        
        csmdebug_print_debug_info("Face table\n");
        i_print_info_debug_faces(solido->sfaces, solido, assert_si_no_es_integro, &num_faces, &num_holes);
        csmdebug_print_debug_info("\n");
        
        csmdebug_print_debug_info("Edge table\n");
        i_print_debug_info_edges(solido->sedges, assert_si_no_es_integro, &num_edges);
        csmdebug_print_debug_info("\n");
        
        csmdebug_print_debug_info("Vertex table\n");
        i_print_debug_info_vertexs(solido->svertexs, assert_si_no_es_integro, &num_vertexs);
        csmdebug_print_debug_info("\n");
        
        csmdebug_end_context();
    }
}

// ----------------------------------------------------------------------------------------------------
#include <basicGraphics/bsgraphics2.h>

static void i_draw_debug_info_vertex(struct csmvertex_t *vertex, struct bsgraphics2_t *graphics)
{
    double x, y, z;
    
    assert_no_null(vertex);
    
    csmvertex_get_coordenadas(vertex, &x, &y, &z);
    bsgraphics2_escr_punto3D(graphics, x, y, z);
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_edge_debug_info(struct csmedge_t *edge, CYBOOL draw_edge_info, struct bsgraphics2_t *graphics)
{
    double x1, y1, z1, x2, y2, z2;
    
    csmedge_vertex_coordinates(edge, &x1, &y1, &z1, &x2, &y2, &z2);
    bsgraphics2_escr_linea3D(graphics, x1, y1, z1, x2, y2, z2);
    
    if (draw_edge_info == CIERTO)
    {
        bsgraphics2_append_desplazamiento_3D(graphics, .5 * (x1 + x2), .5 * (y1 + y2), .5 * (z1 + z2));
        bsgraphics2_append_ejes_plano_pantalla(graphics);
        {
            struct csmhedge_t *he1, *he2;
            char *description;
            
            he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
            he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
        
            description = copiafor_codigo2("+%lu,-%lu", csmhedge_id(he1), csmhedge_id(he2));
            bsgraphics2_escr_texto_mts(graphics, description, 0., 0., 1., 0., BSGRAPHICS2_JUSTIFICACION_CEN_CEN, BSGRAPHICS2_ESTILO_NORMAL, 0.05);
            
            cypestr_destruye(&description);
        }
        bsgraphics2_desapila_transformacion(graphics);
        bsgraphics2_desapila_transformacion(graphics);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_debug_info_edges(struct csmhashtb(csmedge_t) *sedges, CYBOOL draw_edge_info, struct bsgraphics2_t *graphics)
{
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CIERTO)
    {
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, NULL, &edge, csmedge_t);
        i_draw_edge_debug_info(edge, draw_edge_info, graphics);
    }
    
    csmhashtb_free_iterator(&iterator, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_debug_info_vertexs(struct csmhashtb(csmvertex_t) *svertexs, struct bsgraphics2_t *graphics)
{
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    unsigned long num_iters;
    
    iterator = csmhashtb_create_iterator(svertexs, csmvertex_t);
    num_iters = 0;
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CIERTO)
    {
        struct csmvertex_t *vertex;
        
        assert(num_iters < 1000000);
        num_iters++;
        
        csmhashtb_next_pair(iterator, NULL, &vertex, csmvertex_t);
        i_draw_debug_info_vertex(vertex, graphics);
    }
    
    csmhashtb_free_iterator(&iterator, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_draw_debug_info(struct csmsolid_t *solido, CYBOOL draw_edge_info, struct bsgraphics2_t *graphics)
{
    assert_no_null(solido);
    
    csmsolid_redo_geometric_generated_data(solido);
    i_draw_debug_info_vertexs(solido->svertexs, graphics);
    i_draw_debug_info_edges(solido->sedges, draw_edge_info, graphics);
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_solid_faces(
                        struct csmhashtb(csmface_t) *sfaces,
                        CYBOOL draw_solid_face,
                        CYBOOL draw_face_normal,
                        const struct bsmaterial_t *face_material,
                        const struct bsmaterial_t *normal_material,
                        struct bsgraphics2_t *graphics)
{
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    while (csmhashtb_has_next(iterator, csmface_t) == CIERTO)
    {
        struct csmface_t *face;
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        csmface_draw_solid(face, draw_solid_face, draw_face_normal, face_material, normal_material, graphics);
    }
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

static struct csmface_t *i_face_from_hedge(struct csmhedge_t *hedge)
{
    struct csmloop_t *loop;
    
    loop = csmhedge_loop(hedge);
    return csmloop_lface(loop);
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_border_edge(struct csmedge_t *edge, struct bsgraphics2_t *graphics)
{
    struct csmhedge_t *he1, *he2;
    struct csmface_t *face_he1, *face_he2;

    he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    face_he1 = i_face_from_hedge(he1);
    
    he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
    face_he2 = i_face_from_hedge(he2);
    
    if (csmface_faces_define_border_edge(face_he1, face_he2) == CIERTO)
    {
        double x1, y1, z1, x2, y2, z2;
    
        csmedge_vertex_coordinates(edge, &x1, &y1, &z1, &x2, &y2, &z2);
        bsgraphics2_escr_linea3D(graphics, x1, y1, z1, x2, y2, z2);
    }
}

// ----------------------------------------------------------------------------------------------------

static void i_draw_border_edges(struct csmhashtb(csmedge_t) *sedges, struct bsgraphics2_t *graphics)
{
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CIERTO)
    {
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, NULL, &edge, csmedge_t);
        i_draw_border_edge(edge, graphics);
    }
    
    csmhashtb_free_iterator(&iterator, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_draw(
                struct csmsolid_t *solido,
                CYBOOL draw_solid_face,
                CYBOOL draw_face_normal,
                const struct bsmaterial_t *face_material,
                const struct bsmaterial_t *normal_material,
                const struct bsmaterial_t *border_edges_material,
                struct bsgraphics2_t *graphics)
{
    assert_no_null(solido);
    
    csmsolid_redo_geometric_generated_data(solido);
    
    i_draw_solid_faces(solido->sfaces, draw_solid_face, draw_face_normal, face_material, normal_material, graphics);
    
    bsgraphics2_escr_color(graphics, border_edges_material);
    i_draw_border_edges(solido->sedges, graphics);
}

// ----------------------------------------------------------------------------------------------------

static void i_apply_transformation_to_vertexs(
                        struct csmhashtb(csmface_t) *sfaces,
                        struct csmhashtb(csmvertex_t) *svertexs,
                        const struct csmtransform_t *transform)
{
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    
    iterator = csmhashtb_create_iterator(svertexs, csmvertex_t);
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CIERTO)
    {
        struct csmvertex_t *vertex;
        
        csmhashtb_next_pair(iterator, NULL, &vertex, csmvertex_t);
        csmvertex_apply_transform(vertex, transform);
    }
    
    csmhashtb_free_iterator(&iterator, csmvertex_t);
    
    i_redo_geometric_generated_data(sfaces);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_move(struct csmsolid_t *solid, double dx, double dy, double dz)
{
    struct csmtransform_t *transform;
    
    assert_no_null(solid);
    
    transform = csmtransform_make_displacement(dx, dy, dz);
    i_apply_transformation_to_vertexs(solid->sfaces, solid->svertexs, transform);
    
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
    i_apply_transformation_to_vertexs(solid->sfaces, solid->svertexs, transform);
    
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
    
    i_apply_transformation_to_vertexs(solid->sfaces, solid->svertexs, transform);
    
    csmtransform_free(&transform);
}

// ----------------------------------------------------------------------------------------------------
// Note.: Improve using a point for the signed tetrahedra interior to the solid (or as close as possible but out of it).

double csmsolid_volume(const struct csmsolid_t *solid)
{
    double volume;
    struct csmhashtb_iterator(csmface_t) *face_iterator;
    
    assert_no_null(solid);

    i_redo_geometric_generated_data(solid->sfaces);
    
    volume = 0.;
    face_iterator = csmhashtb_create_iterator(solid->sfaces, csmface_t);
    
    while (csmhashtb_has_next(face_iterator, csmface_t) == CIERTO)
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




















