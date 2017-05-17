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

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct i_item_t;
struct csmhashtb(i_item_t);

typedef void (*i_FPtr_reassign_id)(struct i_item_t *item, unsigned long *id_nuevo_elemento, unsigned long *new_id_opc);
#define i_CHECK_FUNC_REASSIGN_ID(function, type) ((void(*)(struct type *, unsigned long *, unsigned long *))function == function)

static CYBOOL i_DEBUG = CIERTO;

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_crea, (
                        unsigned long id_nuevo_elemento,
                        struct csmhashtb(csmface_t) **sfaces,
                        struct csmhashtb(csmedge_t) **sedges,
                        struct csmhashtb(csmvertex_t) **svertexs))
{
    struct csmsolid_t *solido;
    
    solido = MALLOC(struct csmsolid_t);
    
    solido->id_nuevo_elemento = id_nuevo_elemento;
    
    solido->sfaces = ASIGNA_PUNTERO_PP_NO_NULL(sfaces, struct csmhashtb(csmface_t));
    solido->sedges = ASIGNA_PUNTERO_PP_NO_NULL(sedges, struct csmhashtb(csmedge_t));
    solido->svertexs = ASIGNA_PUNTERO_PP_NO_NULL(svertexs, struct csmhashtb(csmvertex_t));
    
    return solido;
}

// ----------------------------------------------------------------------------------------------------

struct csmsolid_t *csmsolid_crea_vacio(void)
{
    unsigned long id_nuevo_elemento;
    struct csmhashtb(csmface_t) *sfaces;
    struct csmhashtb(csmedge_t) *sedges;
    struct csmhashtb(csmvertex_t) *svertexs;
    
    id_nuevo_elemento = 0;
    
    sfaces = csmhashtb_create_empty(csmface_t);
    sedges = csmhashtb_create_empty(csmedge_t);
    svertexs = csmhashtb_create_empty(csmvertex_t);
    
    return i_crea(id_nuevo_elemento, &sfaces, &sedges, &svertexs);
}

// ----------------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_duplicate_solid, (unsigned long id_nuevo_elemento))
{
    struct csmhashtb(csmface_t) *sfaces;
    struct csmhashtb(csmedge_t) *sedges;
    struct csmhashtb(csmvertex_t) *svertexs;
    
    sfaces = csmhashtb_create_empty(csmface_t);
    sedges = csmhashtb_create_empty(csmedge_t);
    svertexs = csmhashtb_create_empty(csmvertex_t);
    
    return i_crea(id_nuevo_elemento, &sfaces, &sedges, &svertexs);
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
    
    new_solid = i_duplicate_solid(solid->id_nuevo_elemento);
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

void csmsolid_remove_face(struct csmsolid_t *solido, struct csmface_t **face)
{
    assert_no_null(solido);
    assert_no_null(face);
    
    csmhashtb_remove_item(solido->sfaces, csmface_id(*face), csmface_t);
    csmface_destruye(face);
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

void csmsolid_remove_vertex(struct csmsolid_t *solido, struct csmvertex_t **vertex)
{
    assert_no_null(solido);
    assert_no_null(vertex);

    csmhashtb_remove_item(solido->svertexs, csmvertex_id(*vertex), csmvertex_t);
    csmvertex_destruye(vertex);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_move_face_to_solid(struct csmsolid_t *face_solid, struct csmface_t *face, struct csmsolid_t *destination_solid)
{
    assert_no_null(face_solid);
    assert_no_null(destination_solid);
    
    if (i_DEBUG == CIERTO)
        fprintf(stdout, "\tcsmsolid_move_face_to_solid(): face: %lu, solid: %p to solid: %p\n", csmface_id(face), csmface_fsolid(face), destination_solid);
    
    csmhashtb_remove_item(face_solid->sfaces, csmface_id(face), csmface_t);
    
    csmface_reassign_id(face, &destination_solid->id_nuevo_elemento, NULL);
    csmhashtb_add_item(destination_solid->sfaces, csmface_id(face), face, csmface_t);
    
    csmface_set_fsolid(face, destination_solid);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_move_edge_to_solid(struct csmsolid_t *edge_solid, struct csmedge_t *edge, struct csmsolid_t *destination_solid)
{
    assert_no_null(edge_solid);
    assert_no_null(destination_solid);
    
    csmhashtb_remove_item(edge_solid->sedges, csmedge_id(edge), csmedge_t);
    
    csmedge_reassign_id(edge, &destination_solid->id_nuevo_elemento, NULL);
    csmhashtb_add_item(destination_solid->sedges, csmedge_id(edge), edge, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_move_vertex_to_solid(struct csmsolid_t *vertex_solid, struct csmvertex_t *vertex, struct csmsolid_t *destination_solid)
{
    assert_no_null(vertex_solid);
    assert_no_null(destination_solid);
    
    csmhashtb_remove_item(vertex_solid->svertexs, csmvertex_id(vertex), csmvertex_t);
    
    csmvertex_reassign_id(vertex, &destination_solid->id_nuevo_elemento, NULL);
    csmhashtb_add_item(destination_solid->svertexs, csmvertex_id(vertex), vertex, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmsolid_contains_edge(const struct csmsolid_t *solid, const struct csmedge_t *edge)
{
    assert_no_null(solid);
    return csmhashtb_contains_id(solid->sedges, csmedge_t, csmedge_id(edge), NULL);
}

// ----------------------------------------------------------------------------------------------------

CYBOOL csmsolid_contains_vertex(const struct csmsolid_t *solid, const struct csmvertex_t *vertex)
{
    assert_no_null(solid);
    return csmhashtb_contains_id(solid->svertexs, csmvertex_t, csmvertex_id(vertex), NULL);
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

struct csmface_t *csmsolid_get_face(struct csmsolid_t *solid, unsigned long id_face)
{
    assert_no_null(solid);
    return csmhashtb_ptr_for_id(solid->sfaces, id_face, csmface_t);
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
    
    csmhashtb_free_iterator(&iterator, csmvertex_t);
    
    return contains_vertex;
}

// ----------------------------------------------------------------------------------------------------

static void i_edge_print_debug_info(struct csmedge_t *edge, CYBOOL assert_si_no_es_integro)
{
    struct csmnode_t *edge_node;
    struct csmhedge_t *he1, *he2;
    
    he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
    he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
    
    edge_node = CSMNODE(edge);
    fprintf(stdout, "\tEdge %5lu", csmnode_id(edge_node));
    
    if (he1 != NULL)
    {
        fprintf(stdout, "\tHe1 %5lu [%d]", csmnode_id(CSMNODE(he1)), ES_CIERTO(csmhedge_edge(he1) == edge));
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_edge(he1) == edge);
    }
    else
    {
        fprintf(stdout, "\tHe1 (null)");
    }
    
    if (he2 != NULL)
    {
        fprintf(stdout, "\tHe2 %5lu [%d]", csmnode_id(CSMNODE(he2)), ES_CIERTO(csmhedge_edge(he2) == edge));
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_edge(he2) == edge);
    }
    else
    {
        fprintf(stdout, "\tHe2 (null)");
    }
    
    fprintf(stdout, "\n");
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
        
        i_edge_print_debug_info(edge, assert_si_no_es_integro);
        (*num_edges)++;
    }
    
    fprintf(stdout, "\tNo. of egdes: %lu\n", *num_edges);
    
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
    
    fprintf(stdout, "\tVertex %6lu\t%6.3lf\t%6.3lf\t%6.3lf ", csmnode_id(vertex_node), x, y, z);
    
    hedge = csmvertex_hedge(vertex);
    
    if (hedge != NULL)
    {
        fprintf(stdout, "He %6lu\n", csmnode_id(CSMNODE(hedge)));
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_vertex(hedge) == vertex);
    }
    else
    {
        fprintf(stdout, "He (null)\n");
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
    
    fprintf(stdout, "\tNo. of vertexs: %lu\n", *num_vertexs);
    
    assert(*num_vertexs == csmhashtb_count(svertexs, csmvertex_t));
    csmhashtb_free_iterator(&iterator, csmvertex_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_print_info_debug_loop(struct csmloop_t *loop, CYBOOL is_outer_loop, CYBOOL assert_si_no_es_integro)
{
    struct csmhedge_t *ledge;
    struct csmhedge_t *iterator;
    unsigned long num_iters;
    
    ledge = csmloop_ledge(loop);
    iterator = ledge;
    fprintf(stdout, "\tLoop %4lu: Outer = %d\n", csmnode_id(CSMNODE(loop)), is_outer_loop);
    
    num_iters = 0;
    
    do
    {
        struct csmvertex_t *vertex;
        double x, y, z;
        struct csmedge_t *edge;
        struct csmhedge_t *next_edge;
        
        assert(num_iters < 10000);
        num_iters++;
        
        vertex = csmhedge_vertex(iterator);
        csmvertex_get_coordenadas(vertex, &x, &y, &z);
        
        edge = csmhedge_edge(iterator);
        
        if (edge == NULL)
        {
            fprintf(
                stdout,
                "\t\t(He %4lu [edge (null)], %6.3f, %6.3f, %6.3f, %d)\n",
                csmnode_id(CSMNODE(iterator)),
                x, y, z,
                ES_CIERTO(csmhedge_loop(iterator) == loop));
        }
        else
        {
            fprintf(
                stdout,
                "\t\t(He %4lu [edge %6lu], %6.3f, %6.3f, %6.3f, %d)\n",
                csmnode_id(CSMNODE(iterator)),
                csmnode_id(CSMNODE(edge)),
                x, y, z,
                ES_CIERTO(csmhedge_loop(iterator) == loop));
        }
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_loop(iterator) == loop);
        
        next_edge = csmhedge_next(iterator);
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmhedge_prev(next_edge) == iterator);
                    
        iterator = next_edge;
    }
    while (iterator != ledge);
}

// ----------------------------------------------------------------------------------------------------

static void i_print_info_debug_face(struct csmface_t *face, CYBOOL assert_si_no_es_integro, unsigned long *num_holes)
{
    struct csmloop_t *loop_iterator;
    
    assert_no_null(num_holes);
    
    fprintf(stdout, "\tFace %lu\n", csmnode_id(CSMNODE(face)));
    
    loop_iterator = csmface_floops(face);
    
    while (loop_iterator != NULL)
    {
        struct csmloop_t *next_loop;
        CYBOOL is_outer_loop;
        
        is_outer_loop = ES_CIERTO(csmface_flout(face) == loop_iterator);
        i_print_info_debug_loop(loop_iterator, is_outer_loop, assert_si_no_es_integro);
        
        if (is_outer_loop == FALSO)
            (*num_holes)++;
        
        next_loop = csmloop_next(loop_iterator);
        
        if (assert_si_no_es_integro == CIERTO)
        {
            assert(csmloop_lface(loop_iterator) == face);
            
            if (next_loop != NULL)
                assert(csmloop_prev(next_loop) == loop_iterator);
        }
        
        loop_iterator = next_loop;
    }
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
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        
        i_print_info_debug_face(face, assert_si_no_es_integro, num_holes);
        (*num_faces)++;
        
        if (assert_si_no_es_integro == CIERTO)
            assert(csmface_fsolid(face) == solid);
    }
    
    fprintf(stdout, "\tNo. of faces: %lu\n", *num_faces);
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_print_debug(struct csmsolid_t *solido, CYBOOL assert_si_no_es_integro)
{
    unsigned long num_faces, num_vertexs, num_edges, num_holes;
    
    assert_no_null(solido);

    fprintf(stdout, "*******************************\n");
    fprintf(stdout, "*******************************\n");
    fprintf(stdout, "Face table\n");
    i_print_info_debug_faces(solido->sfaces, solido, assert_si_no_es_integro, &num_faces, &num_holes);
    fprintf(stdout, "\n");
    
    fprintf(stdout, "Edge table\n");
    i_print_debug_info_edges(solido->sedges, assert_si_no_es_integro, &num_edges);
    fprintf(stdout, "\n");
    
    fprintf(stdout, "Vertex table\n");
    i_print_debug_info_vertexs(solido->svertexs, assert_si_no_es_integro, &num_vertexs);
    fprintf(stdout, "\n");
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
            struct csmhedge_t *loop_ledge;
            register struct csmhedge_t *he_iterator;
            double x1, y1, z1;
            unsigned long num_he_iterations;
            
            assert(num_loop_iterations < 1000);
            num_loop_iterations++;
            
            loop_ledge = csmloop_ledge(loop_iterator);
            csmvertex_get_coordenadas(csmhedge_vertex(loop_ledge), &x1, &y1, &z1);
            
            he_iterator = loop_ledge;
            num_he_iterations = 0;
            
            do
            {
                double x2, y2, z2;
                double Ux_cross, Uy_cross, Uz_cross;
                struct csmhedge_t *next_hedge;
                double x_next, y_next, z_next;
                
                assert(num_he_iterations < 10000);
                num_he_iterations++;
                
                csmvertex_get_coordenadas(csmhedge_vertex(he_iterator), &x2, &y2, &z2);
                csmmath_cross_product3D(x1, y1, z1, x2, y2, z2, &Ux_cross, &Uy_cross, &Uz_cross);
                
                next_hedge = csmhedge_next(he_iterator);
                csmvertex_get_coordenadas(csmhedge_vertex(next_hedge), &x_next, &y_next, &z_next);
                
                volume += csmmath_dot_product3D(x_next, y_next, z_next, Ux_cross, Uy_cross, Uz_cross);
                
            }
            while (he_iterator != loop_ledge);
            
            loop_iterator = csmloop_next(loop_iterator);
        }
    }
    
    csmhashtb_free_iterator(&face_iterator, csmface_t);
    
    return volume / 6.;
}




















