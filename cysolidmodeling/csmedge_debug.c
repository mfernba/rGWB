//
//  csmedge_debug.c
//  cysolidmodeling
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmedge_debug.inl"
#include "csmedge.tli"

#include "csmassert.inl"
#include "csmdebug.inl"
#include "csmhedge.inl"
#include "csmvertex.inl"

// ----------------------------------------------------------------------------------------------------

void csmedge_debug_print_debug_info(struct csmedge_t *edge, CSMBOOL assert_si_no_es_integro)
{
    struct csmhedge_t *he1, *he2;
    double x1, y1, z1, x2, y2, z2;
    
    assert_no_null(edge);
    
    he1 = edge->he1;
    he2 = edge->he2;
    
    csmdebug_print_debug_info("\tEdge %5lu", edge->id);
    
    if (he1 != NULL)
    {
        const struct csmvertex_t *vertex;
        
        csmdebug_print_debug_info("\tHe1 %5lu [%d]", csmhedge_id(he1), IS_TRUE(csmhedge_edge(he1) == edge));
        
        if (assert_si_no_es_integro == CSMTRUE)
            assert(csmhedge_edge(he1) == edge);
        
        vertex = csmhedge_vertex_const(he1);
        csmvertex_get_coordenadas(vertex, &x1, &y1, &z1);
    }
    else
    {
        csmdebug_print_debug_info("\tHe1 (null)");
        x1 = y1 = z1 = 0.;
    }
    
    if (he2 != NULL)
    {
        const struct csmvertex_t *vertex;
        
        csmdebug_print_debug_info("\tHe2 %5lu [%d]", csmhedge_id(he2), IS_TRUE(csmhedge_edge(he2) == edge));
        
        if (assert_si_no_es_integro == CSMTRUE)
            assert(csmhedge_edge(he2) == edge);
        
        vertex = csmhedge_vertex_const(he2);
        csmvertex_get_coordenadas(vertex, &x2, &y2, &z2);
    }
    else
    {
        csmdebug_print_debug_info("\tHe2 (null)");
        x2 = y2 = z2 = 0.;
    }
    
    if (he1 != NULL && he2 != NULL )
        csmdebug_print_debug_info("\t(%5.3lf, %5.3lf, %5.3lf)\t(%5.3lf, %5.3lf, %5.3lf)", x1, y1, z1, x2, y2, z2);
    
    csmdebug_print_debug_info("\n");
}


