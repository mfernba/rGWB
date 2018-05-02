// Convenience operator. Moves a loop from one face to other...

#include "csmeuler_lringmv.inl"

#include "csmface.inl"
#include "csmloop.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

// --------------------------------------------------------------------------------

void csmeuler_lringmv(struct csmloop_t *loop, struct csmface_t *face, CSMBOOL as_outer_loop)
{
    struct csmface_t *old_face;
    
    old_face = csmloop_lface(loop);
    
    if (old_face != face)
        csmface_add_loop_while_removing_from_old(face, loop);
    
    if (as_outer_loop == CSMTRUE)
        csmface_set_flout(face, loop);
}

// ------------------------------------------------------------------------------------------

void csmeuler_lringmv_move_all_loops_from_face1_to_face2(struct csmface_t *face1, struct csmface_t *face2)
{
    register struct csmloop_t *iterator_face1;
    unsigned long num_iteraciones;
    
    csmface_redo_geometric_generated_data(face1);
    
    iterator_face1 = csmface_floops(face1);
    num_iteraciones = 0;
    
    do
    {
        struct csmloop_t *next_loop;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        next_loop = csmloop_next(iterator_face1);
        csmface_add_loop_while_removing_from_old(face2, iterator_face1);
        
        iterator_face1 = next_loop;
        
    } while (iterator_face1 != NULL);
}
