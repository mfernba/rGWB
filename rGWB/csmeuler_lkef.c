// low level kill edge-face...
//
// Removes the edge of he1 and he2 and "joins" the two adjacent faces by merging their loops.
// The face of he2 is removed.
//
// lkef is applicable to the halves of an edge that occurs in two distinct faces.
// So, he1->edge  == he2->edge and he1->face != he2->face


#include "csmeuler_lkef.inl"

#include "csmassert.inl"
#include "csmedge.tli"
#include "csmface.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmmem.inl"
#include "csmnode.inl"
#include "csmopbas.inl"
#include "csmsolid.inl"
#include "csmvertex.inl"

// --------------------------------------------------------------------------------

static void i_move_all_loops_from_he2_face1_to_he1_face(struct csmhedge_t *he1, struct csmhedge_t *he2)
{
    struct csmface_t *face_he1, *face_he2;
    register struct csmloop_t *loop_iterator_face2;
    register unsigned long num_iteraciones;

    assert(he1 != he2);
    assert(csmhedge_edge(he1) == csmhedge_edge(he2));
    
    face_he1 = csmopbas_face_from_hedge(he1);
    face_he2 = csmopbas_face_from_hedge(he2);
    assert(face_he1 != face_he2);
    
    loop_iterator_face2 = csmface_floops(face_he2);
    num_iteraciones = 0;
    
    do
    {
        struct csmloop_t *loop_to_move;
        
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        loop_to_move = loop_iterator_face2;
        loop_iterator_face2 = csmloop_next(loop_iterator_face2);
        
        csmface_add_loop_while_removing_from_old(face_he1, loop_to_move);
        
    } while (loop_iterator_face2 != NULL);
}

// --------------------------------------------------------------------------------

static void i_merge_halfegdes_loops_isolating_edge(struct csmhedge_t **he1, struct csmhedge_t **he2)
{
    struct csmsolid_t *hes_solid;
    struct csmedge_t *common_edge;
    struct csmloop_t *he1_loop, *he2_loop;
    struct csmhedge_t *prev_he1, *next_he1;
    struct csmhedge_t *prev_he2, *next_he2;
    register struct csmhedge_t *he_iterator;
    register unsigned long num_iteraciones;
    struct csmface_t *he2_loop_face;
    
    assert_no_null(he1);
    assert_no_null(he2);
    assert(*he1 != *he2);
    
    hes_solid = csmopbas_solid_from_hedges(*he1, *he2);
    
    common_edge = csmhedge_edge(*he1);
    assert(common_edge == csmhedge_edge(*he2));
    
    he1_loop = csmhedge_loop(*he1);
    he2_loop = csmhedge_loop(*he2);
    he_iterator = csmloop_ledge(he2_loop);
    num_iteraciones = 0;
    
    do
    {
        assert(num_iteraciones < 10000);
        num_iteraciones++;
        
        csmhedge_set_loop(he_iterator, he1_loop);
        he_iterator = csmhedge_next(he_iterator);
        
    } while (he_iterator != csmloop_ledge(he2_loop));
    
    prev_he1 = csmhedge_prev(*he1);
    next_he1 = csmhedge_next(*he1);
    
    prev_he2 = csmhedge_prev(*he2);
    next_he2 = csmhedge_next(*he2);
    
    csmloop_set_ledge(he1_loop, next_he1);
    
    csmvertex_set_hedge(csmhedge_vertex(*he1), next_he2);
    csmvertex_set_hedge(csmhedge_vertex(*he2), next_he1);
    
    csmhedge_set_next(prev_he2, next_he1);
    csmhedge_set_prev(next_he1, prev_he2);
    
    csmhedge_set_next(prev_he1, next_he2);
    csmhedge_set_prev(next_he2, prev_he1);

    csmloop_set_ledge(he2_loop, NULL);
    he2_loop_face = csmloop_lface(he2_loop);
    csmface_remove_loop(he2_loop_face, &he2_loop);
    
    csmhedge_set_loop(*he1, NULL);
    csmhedge_set_next(*he1, NULL);
    csmhedge_set_prev(*he1, NULL);

    csmhedge_set_loop(*he2, NULL);
    csmhedge_set_next(*he2, NULL);
    csmhedge_set_prev(*he2, NULL);
    
    csmopbas_delhe(he2, NULL, NULL);
    csmopbas_delhe(he1, NULL, NULL);
    csmsolid_remove_edge(hes_solid, &common_edge);
    
    *he1 = next_he1;
    *he2 = next_he2;
}

// --------------------------------------------------------------------------------

void csmeuler_lkef(struct csmhedge_t **he1, struct csmhedge_t **he2)
{
    struct csmface_t *old_he2_face;
    struct csmsolid_t *face_solid;
    
    assert_no_null(he1);
    assert_no_null(he2);
    assert(*he1 != *he2);

    old_he2_face = csmopbas_face_from_hedge(*he2);
    
    i_move_all_loops_from_he2_face1_to_he1_face(*he1, *he2);
    i_merge_halfegdes_loops_isolating_edge(he1, he2);
    
    face_solid = csmface_fsolid(old_he2_face);
    csmsolid_remove_face(face_solid, &old_he2_face);
}








