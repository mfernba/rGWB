// low make face kill ring hole...
//
// Makes the loop the outer loop of a new face...
// It's assumed that loop is an inner loop of its parent face

#include "csmeuler_lmfkrh.inl"

#include "csmface.inl"
#include "csmloop.inl"
#include "csmsolid.inl"

#include "cyassert.h"
#include "cypespy.h"

// --------------------------------------------------------------------------------

void csmeuler_lmfkrh(struct csmloop_t *loop, unsigned long *id_nuevo_elemento, struct csmface_t **new_face_opc)
{
    struct csmface_t *new_face_loc;
    struct csmface_t *old_face;
    struct csmsolid_t *solid;
    
    old_face = csmloop_lface(loop);
    assert(csmface_flout(old_face) != loop);
    
    solid = csmface_fsolid(old_face);
    
    csmsolid_append_new_face(solid, id_nuevo_elemento, &new_face_loc);
    csmface_add_loop_while_removing_from_old(new_face_loc, loop);
    
    ASIGNA_OPC(new_face_opc, new_face_loc);
}
