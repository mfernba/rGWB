//
//  csmloop_debug.c
//  cysolidmodeling
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmloop_debug.inl"
#include "csmloop.tli"

#include "csmassert.inl"
#include "csmdebug.inl"
#include "csmedge.inl"
#include "csmedge.tli"
#include "csmnode.inl"
#include "csmhedge.inl"
#include "csmstring.inl"
#include "csmvertex.inl"

// ----------------------------------------------------------------------------------------------------

void csmloop_debug_print_info_debug(
	                    struct csmloop_t *loop,
                        CSMBOOL is_outer_loop,
                        CSMBOOL with_loop_area, double loop_area,
                        CSMBOOL assert_si_no_es_integro)
{
    struct csmhedge_t *ledge;
    struct csmhedge_t *iterator;
    unsigned long num_iters;
    
    assert_no_null(loop);
    
    ledge = loop->ledge;
    iterator = ledge;
    
    if (with_loop_area == CSMTRUE)
        csmdebug_print_debug_info("\tLoop %4lu: Outer = %d Area = %lf\n", csmnode_id(CSMNODE(loop)), is_outer_loop, loop_area);
    else
        csmdebug_print_debug_info("\tLoop %4lu: Outer = %d\n", csmnode_id(CSMNODE(loop)), is_outer_loop);
    
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
            csmdebug_print_debug_info(
                "\t\t(He %4lu [edge (null)], %4lu, %6.3f, %6.3f, %6.3f, %d)\n",
                csmnode_id(CSMNODE(iterator)),
                csmnode_id(CSMNODE(vertex)),
                x, y, z,
                IS_TRUE(csmhedge_loop(iterator) == loop));
        }
        else
        {
            char *is_null_edge;
            const char *he_position;
            struct csmhedge_t *he1, *he2;
            struct csmhedge_t *he_mate;
            
            if (csmedge_setop_is_null_edge(edge) == CSMTRUE)
                is_null_edge = copiafor_codigo1("[Null Edge: %lu]", csmedge_id(edge));
            else
                is_null_edge = csmstring_duplicate("");
            
            he1 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_POS);
            he2 = csmedge_hedge_lado(edge, CSMEDGE_LADO_HEDGE_NEG);
            he_mate = (iterator == he1) ? he2: he1;
            he_position = (iterator == he1) ? "HE1": "HE2";
            
            if (he_mate != NULL)
            {
                csmdebug_print_debug_info(
                    "\t\t(%3s %4lu [edge %6lu. Mate: %4lu], %4lu, %6.3f, %6.3f, %6.3f, %d) %s\n",
                    he_position,
                    csmnode_id(CSMNODE(iterator)),
                    csmnode_id(CSMNODE(edge)),
                    csmnode_id(CSMNODE(he_mate)),
                    csmnode_id(CSMNODE(vertex)),
                    x, y, z,
                    IS_TRUE(csmhedge_loop(iterator) == loop),
                    is_null_edge);
            }
            else
            {
                csmdebug_print_debug_info(
                    "\t\t(%3s %4lu [edge %6lu. Mate: ----], %4lu, %6.3f, %6.3f, %6.3f, %d) %s\n",
                    he_position,
                    csmnode_id(CSMNODE(iterator)),
                    csmnode_id(CSMNODE(edge)),
                    csmnode_id(CSMNODE(vertex)),
                    x, y, z,
                    IS_TRUE(csmhedge_loop(iterator) == loop),
                    is_null_edge);
            }
            
            csmstring_free(&is_null_edge);
        }
        
        if (assert_si_no_es_integro == CSMTRUE)
            assert(csmhedge_loop(iterator) == loop);
        
        next_edge = csmhedge_next(iterator);
        
        if (assert_si_no_es_integro == CSMTRUE)
            assert(csmhedge_prev(next_edge) == iterator);
        
        iterator = next_edge;
    }
    while (iterator != ledge);
}
