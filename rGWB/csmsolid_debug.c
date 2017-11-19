//
//  csmsolid_debug.c
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmsolid_debug.inl"
#include "csmsolid.tli"

#include "csmassert.inl"
#include "csmdebug.inl"
#include "csmhashtb.inl"
#include "csmedge.inl"
#include "csmedge_debug.inl"
#include "csmface.inl"
#include "csmface_debug.inl"
#include "csmhedge.inl"
#include "csmnode.inl"
#include "csmvertex.inl"
#include "csmvertex_debug.inl"

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_edges(struct csmhashtb(csmedge_t) *sedges, CSMBOOL assert_si_no_es_integro, unsigned long *num_edges)
{
    struct csmhashtb_iterator(csmedge_t) *iterator;
    
    assert_no_null(num_edges);
    
    iterator = csmhashtb_create_iterator(sedges, csmedge_t);
    *num_edges = 0;
    
    while (csmhashtb_has_next(iterator, csmedge_t) == CSMTRUE)
    {
        struct csmedge_t *edge;
        
        csmhashtb_next_pair(iterator, NULL, &edge, csmedge_t);
        
        csmedge_debug_print_debug_info(edge, assert_si_no_es_integro);
        (*num_edges)++;
    }
    
    csmdebug_print_debug_info("\tNo. of edges: %lu\n", *num_edges);
    
    assert(*num_edges == csmhashtb_count(sedges, csmedge_t));
    csmhashtb_free_iterator(&iterator, csmedge_t);
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_vertex(struct csmvertex_t *vertex, CSMBOOL assert_si_no_es_integro)
{
    struct csmhedge_t *hedge;
    
    csmvertex_debug_print_debug_info(vertex);
    
    hedge = csmvertex_hedge(vertex);
    
    if (hedge != NULL)
    {
        csmdebug_print_debug_info("He %6lu", csmnode_id(CSMNODE(hedge)));
        
        if (assert_si_no_es_integro == CSMTRUE)
            assert(csmhedge_vertex(hedge) == vertex);
    }
    else
    {
        csmdebug_print_debug_info("He (null)");
    }
    
    csmdebug_print_debug_info("\n");
}

// ----------------------------------------------------------------------------------------------------

static void i_print_debug_info_vertexs(struct csmhashtb(csmvertex_t) *svertexs, CSMBOOL assert_si_no_es_integro, unsigned long *num_vertexs)
{
    struct csmhashtb_iterator(csmvertex_t) *iterator;
    unsigned long num_iters;
    
    assert_no_null(num_vertexs);
    
    iterator = csmhashtb_create_iterator(svertexs, csmvertex_t);
    *num_vertexs = 0;
    
    num_iters = 0;
    
    while (csmhashtb_has_next(iterator, csmvertex_t) == CSMTRUE)
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
                        CSMBOOL assert_si_no_es_integro,
                        unsigned long *num_faces, unsigned long *num_holes)
{
    struct csmhashtb_iterator(csmface_t) *iterator;
    
    assert_no_null(num_faces);
    assert_no_null(num_holes);
    
    iterator = csmhashtb_create_iterator(sfaces, csmface_t);
    
    *num_faces = 0;
    *num_holes = 0;
    
    while (csmhashtb_has_next(iterator, csmface_t) == CSMTRUE)
    {
        struct csmface_t *face;
        unsigned long num_holes_loc;
        
        csmhashtb_next_pair(iterator, NULL, &face, csmface_t);
        
        csmface_debug_print_info_debug(face, assert_si_no_es_integro, &num_holes_loc);
        (*num_faces)++;
        *num_holes += num_holes_loc;
        
        if (assert_si_no_es_integro == CSMTRUE)
            assert(csmface_fsolid(face) == solid);
    }
    
    csmdebug_print_debug_info("\tNo. of faces: %lu\n", *num_faces);
    
    csmhashtb_free_iterator(&iterator, csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_debug_print_debug(struct csmsolid_t *solido, CSMBOOL assert_si_no_es_integro)
{
    if (csmdebug_debug_enabled() == CSMTRUE && csmdebug_is_print_solid_unblocked() == CSMTRUE)
    {
        unsigned long num_faces, num_holes;
        
        assert_no_null(solido);
        
        csmdebug_begin_context("SOLID DESCRIPTION");
        
        if (solido->name != NULL)
            csmdebug_print_debug_info("Solid Address: %s (%p)\n", solido->name, solido);
        else
            csmdebug_print_debug_info("Solid Address: %p\n", solido);
        
        csmdebug_print_debug_info("Face table\n");
        i_print_info_debug_faces(solido->sfaces, solido, assert_si_no_es_integro, &num_faces, &num_holes);
        csmdebug_print_debug_info("\n");
        
        /*
        csmdebug_print_debug_info("Edge table\n");
        i_print_debug_info_edges(solido->sedges, assert_si_no_es_integro, &num_edges);
        csmdebug_print_debug_info("\n");
        
        csmdebug_print_debug_info("Vertex table\n");
        i_print_debug_info_vertexs(solido->svertexs, assert_si_no_es_integro, &num_vertexs);
        csmdebug_print_debug_info("\n");
        */
        
        csmdebug_end_context();
    }
}

// ----------------------------------------------------------------------------------------------------

void csmsolid_debug_print_complete_debug(struct csmsolid_t *solido, CSMBOOL assert_si_no_es_integro)
{
    if (csmdebug_debug_enabled() == CSMTRUE && csmdebug_is_print_solid_unblocked() == CSMTRUE)
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