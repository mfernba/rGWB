// Convenience operator. Moves a loop from one face to other...

#include "csmeuler_lringmv.inl"

#include "csmface.inl"
#include "csmloop.inl"
#include "csmsolid.inl"

#include "csmassert.inl"
#include "csmmem.inl"

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
