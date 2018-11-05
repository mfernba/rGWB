// low make face kill ring hole...
//
// Makes the loop the outer loop of a new face...
// It's assumed that loop is an inner loop of its parent face

#include "csmeuler_lmfkrh.inl"

#include "csmface.inl"
#include "csmloop.inl"
#include "csmsolid.inl"

#ifdef RGWB_STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

// --------------------------------------------------------------------------------

void csmeuler_lmfkrh(struct csmloop_t *loop, struct csmface_t **new_face_opc)
{
    struct csmface_t *new_face_loc;
    struct csmface_t *old_face;
    struct csmsolid_t *solid;
    
    old_face = csmloop_lface(loop);
    assert(csmface_flout(old_face) != loop);
    
    solid = csmface_fsolid(old_face);
    
    csmsolid_append_new_face(solid, &new_face_loc);
    csmface_copy_attributes_from_face1(old_face, new_face_loc);
    
    csmface_add_loop_while_removing_from_old(new_face_loc, loop);
    
    ASSIGN_OPTIONAL_VALUE(new_face_opc, new_face_loc);
}
