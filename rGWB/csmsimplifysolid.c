//
//  csmsimplifysolid.c
//  rGWB
//
//  Created by Manuel Fernández on 19/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmsimplifysolid.inl"

#include "csmarrayc.inl"
#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmeuler_lkef.inl"
#include "csmeuler_lkev.inl"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmopbas.inl"
#include "csmtolerance.inl"
#include "csmsolid.inl"
#include "csmsolid_debug.inl"
#include "csmvertex.inl"
#include "csmvertex.tli"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

// ----------------------------------------------------------------------------------------------------

static void i_delete_redundant_faces(struct csmsolid_t *solid, const struct csmtolerance_t *tolerances)
{
    CSMBOOL there_are_changes;
    unsigned long no_iters;
    
    no_iters = 0;
    
    do
    {
        struct csmhashtb_iterator(csmedge_t) *edge_iterator;
        
        assert(no_iters < 10000);
        no_iters++;
        
        edge_iterator = csmsolid_edge_iterator(solid);
        there_are_changes = CSMFALSE;
        
        while (csmhashtb_has_next(edge_iterator, csmedge_t) == CSMTRUE)
        {
            struct csmedge_t *edge;
            struct csmhedge_t *he1, *he2;
            struct csmface_t *face_he1, *face_he2;
            CSMBOOL same_sense;
            
            csmhashtb_next_pair(edge_iterator, NULL, &edge, csmedge_t);
            
            he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
            face_he1 = csmopbas_face_from_hedge(he1);
            
            he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
            face_he2 = csmopbas_face_from_hedge(he2);
    
            if (face_he1 != face_he2
                    && csmface_are_coplanar_faces(face_he1, face_he2, tolerances, &same_sense) == CSMTRUE
                    && same_sense == CSMTRUE)
            {
                csmeuler_lkef(&he1, &he2);
                there_are_changes = CSMTRUE;
                break;
            }
        }
        
        csmhashtb_free_iterator(&edge_iterator, csmedge_t);
        
    } while (there_are_changes == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

static unsigned long i_num_uses_of_vertex(struct csmvertex_t *vertex)
{
    unsigned long no_uses;
    struct csmhedge_t *he, *he_iterator;
    unsigned long no_iterations;
    
    he = csmvertex_hedge(vertex);
    he_iterator = he;
    
    no_uses = 0;
    no_iterations = 0;
    
    do
    {
        assert(no_iterations < 1000);
        no_iterations++;
        
        no_uses++;
        he_iterator = csmhedge_next(csmopbas_mate(he_iterator));
        
    } while (he != he_iterator);
    
    return no_uses;
}

// ----------------------------------------------------------------------------------------------------

static void i_delete_redundant_vertexs(struct csmsolid_t *solid)
{
    CSMBOOL there_are_changes;
    unsigned long no_iterations;
    
    no_iterations = 0;
    
    do
    {
        struct csmhashtb_iterator(csmvertex_t) *vertex_iterator;
        
        assert(no_iterations < 100000);
        no_iterations++;
        
        vertex_iterator = csmsolid_vertex_iterator(solid);
        there_are_changes = CSMFALSE;
        
        while (csmhashtb_has_next(vertex_iterator, csmvertex_t) == CSMTRUE)
        {
            struct csmvertex_t *vertex;
            
            csmhashtb_next_pair(vertex_iterator, NULL, &vertex, csmvertex_t);
            
            if (i_num_uses_of_vertex(vertex) == 2)
            {
                struct csmhedge_t *he1, *he2;
                
                he1 = csmvertex_hedge(vertex);
                he2 = csmhedge_next(csmopbas_mate(he1));
                
                if (csmhedge_edge(he1) != csmhedge_edge(he2))
                {
                    struct csmhedge_t *he1_mate;
                    
                    he1_mate = csmopbas_mate(he1);
                    
                    csmeuler_lkev(&he1, &he1_mate, NULL, NULL, NULL, NULL);
                    there_are_changes = CSMTRUE;
                    
                    csmsolid_debug_print_debug(solid, CSMTRUE);
                    break;
                }
            }
        }
        
        csmhashtb_free_iterator(&vertex_iterator, csmvertex_t);
        
    } while (there_are_changes == CSMTRUE);
}

// ----------------------------------------------------------------------------------------------------

void csmsimplifysolid_simplify(struct csmsolid_t *solid)
{
    /*
    struct csmtolerance_t *tolerances;
    
    tolerances = csmtolerance_new();
    
    csmsolid_redo_geometric_face_data(solid);
    csmdebug_print_debug_info("Before simplification...\n");
    csmsolid_debug_print_debug(solid, CSMTRUE);
    
    csmsolid_debug_print_debug(solid, CSMTRUE);
    
    csmdebug_print_debug_info("Deleting coplanar faces...\n");
    i_delete_redundant_faces(solid, tolerances);
    
    csmdebug_print_debug_info("Deleting vertexs...\n");
    i_delete_redundant_vertexs(solid);

    csmdebug_print_debug_info("After simplification...\n");
    csmsolid_debug_print_debug(solid, CSMTRUE);
    
    csmtolerance_free(&tolerances);
    */
}
