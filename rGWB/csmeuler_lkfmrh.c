// low kill face make ring hole...
//
// Merges two faces f1, f2 by making the loop of the latter a ring into the former. Face2 is removed and
// has only 1 loop.
//

#include "csmeuler_lkfmrh.inl"

#include "csmassert.inl"
#include "csmface.inl"
#include "csmloop.inl"
#include "csmmem.inl"
#include "csmsolid.inl"

// --------------------------------------------------------------------------------

void csmeuler_lkfmrh(struct csmface_t *face_to_add_loop, struct csmface_t **face_to_remove)
{
    struct csmface_t *face_to_remove_loc;
    struct csmsolid_t *face_to_remove_solid;
    struct csmloop_t *loop_iterator;
    unsigned long num_iteraciones;
    
    face_to_remove_loc = ASIGNA_PUNTERO_PP_NO_NULL(face_to_remove, struct csmface_t);
    face_to_remove_solid = csmface_fsolid(face_to_remove_loc);
    
    loop_iterator = csmface_floops(face_to_remove_loc);
    num_iteraciones = 0;
    
    while (loop_iterator != NULL)
    {
        struct csmloop_t *next_loop;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        next_loop = csmloop_next(loop_iterator);
        csmface_add_loop_while_removing_from_old(face_to_add_loop, loop_iterator);
        
        loop_iterator = next_loop;
    }
    
    csmface_set_flout(face_to_remove_loc, NULL);
    csmface_set_floops(face_to_remove_loc, NULL);
    csmsolid_remove_face(face_to_remove_solid, &face_to_remove_loc);
}
