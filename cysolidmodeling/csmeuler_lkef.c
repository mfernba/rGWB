// low level kill edge-face...
//
// Removes the edge of he1 and he2 and "joins" the two adjacent faces by merging their loops.
// The face of he2 is removed.
//
// lkef is applicable to the halves of an edge that occurs in two distinct faces.
// So, he1->edge  == he2->edge and he1->face != he2->face


#include "csmeuler_lkef.inl"

#include "csmedge.tli"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmnode.inl"
#include "csmopbas.inl"
#include "csmsolid.inl"
#include "csmvertex.inl"

#include "cyassert.h"
#include "cypespy.h"

// --------------------------------------------------------------------------------

void csmeuler_lkef(struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    /*
    struct csmhedge_t *he_iterator;
    struct csmhedge_t *prev_he1, *next_he1;
    struct csmhedge_t *prev_he2, *next_he2;
    struct csmface_t *he2_face;
    
    {
        struct csmloop_t *he1_loop, *he2_loop;
        struct csmface_t *face_he1, *face_he2;
        struct csmloop_t *loop_iterator_face1;
        
        he1_loop = csmhedge_loop(he1);
        face_he1 = csmloop_lface(he1_loop);

        he2_loop = csmhedge_loop(he2);
        face_he2 = csmloop_lface(he2_loop);
        
        loop_iterator_face1 = csmface_floops(face_he1);
        
        do
        {
            struct csmloop_t *loop_to_move;
            
            loop_to_move = loop_iterator_face1;
            loop_iterator_face1 = csmloop_next(loop_iterator_face1);
            
            csmloop_set_face(loop_to_move, face_he2);
            csmface_add_loop(face_he2, loop_to_move);
            
        } while (loop_iterator_face1 != NULL);
    }
    

    
    he_iterator = csmhedge_next(he2);
    while (he_iterator != he2)
    {
        csmhedge_set_loop(he_iterator, he1_loop);
        he_iterator = csmhedge_next(he2);
    }
    
    prev_he1 = csmhedge_prev(he1);
    next_he1 = csmhedge_next(he1);
    
    prev_he2 = csmhedge_prev(he2);
    next_he2 = csmhedge_next(he2);
    
    csmhedge_set_next(prev_he2, next_he1);
    csmhedge_set_prev(next_he1, prev_he2);
    
    csmhedge_set_next(prev_he1, next_he2);
    csmhedge_set_prev(next_he2, prev_he1);
    
    he2_face = csmloop_lface(csmhedge_loop(he2));
    csmnode_release_ex_checking_must_be_destroyed(&he2_face, csmface_t);
    */
}








