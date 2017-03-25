// Face...

#include "csmface.inl"

#include "csmbbox.inl"
#include "csmloop.inl"
#include "csmnode.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct csmface_t
{
    unsigned long id;

    struct csmsolid_t *fsolid;
    
    struct csmloop_t *flout;
    struct csmloop_t *floops;
    
    double A, B, C, D;
    struct csmbbox_t *bbox;
};

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmface_t *, i_crea, (
                        unsigned long id,
                        struct csmsolid_t *fsolid,
                        struct csmloop_t *flout,
                        struct csmloop_t *floops,
                        double A, double B, double C, double D,
                        struct csmbbox_t **bbox))
{
    struct csmface_t *face;
    
    face = MALLOC(struct csmface_t);
    
    face->id = id;
    
    face->fsolid = fsolid;
    
    face->flout = flout;
    face->floops = floops;
    
    face->A = A;
    face->B = B;
    face->C = C;
    face->D = D;
    
    face->bbox = ASIGNA_PUNTERO_PP_NO_NULL(bbox, struct csmbbox_t);
    
    return face;
}

// ------------------------------------------------------------------------------------------

struct csmface_t *csmface_crea(struct csmsolid_t *solido, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmsolid_t *fsolid;
    struct csmloop_t *flout;
    struct csmloop_t *floops;
    double A, B, C, D;
    struct csmbbox_t *bbox;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);

    fsolid = solido;
    flout = NULL;
    floops = NULL;
    
    A = 0.;
    B = 0.;
    C = 0.;
    D = 0.;
    
    bbox = csmbbox_crea_vacia();
    
    return i_crea(id, fsolid, flout, floops, A, B, C, D, &bbox);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmface_t *, i_duplicate_face, (
                        struct csmsolid_t *fsolid,
                        double A, double B, double C, double D,
                        unsigned long *id_nuevo_elemento))
{
    unsigned long id;
    struct csmloop_t *flout, *floops;
    struct csmbbox_t *bbox;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);
    flout = NULL;
    floops = NULL;
    bbox = csmbbox_crea_vacia();
    
    return i_crea(id, fsolid, flout, floops, A, B, C, D, &bbox);
}

// ------------------------------------------------------------------------------------------

struct csmface_t *csmface_duplicate(
                        struct csmface_t *face,
                        struct csmsolid_t *fsolid,
                        unsigned long *id_nuevo_elemento,
                        struct csmhashtb(csmvertex_t) *relation_svertexs_old_to_new,
                        struct csmhashtb(csmhedge_t) *relation_shedges_old_to_new)
{
    struct csmface_t *new_face;
    struct csmloop_t *iterator, *last_loop;
    
    assert_no_null(face);

    new_face = i_duplicate_face(fsolid, face->A, face->B, face->C, face->D, id_nuevo_elemento);
    assert_no_null(new_face);
    
    iterator = face->floops;
    last_loop = NULL;
    
    while (iterator != NULL)
    {
        struct csmloop_t *iterator_copy;
        
        iterator_copy = csmloop_duplicate(
                        iterator,
                        new_face,
                        id_nuevo_elemento,
                        relation_svertexs_old_to_new,
                        relation_shedges_old_to_new);
        
        if (new_face->floops == NULL)
            new_face->floops = iterator_copy;
        else
            csmnode_insert_node2_after_node1(last_loop, iterator_copy, csmloop_t);
        
        if (iterator == face->flout)
            new_face->flout = iterator_copy;
        
        last_loop = iterator_copy;
        iterator = csmloop_next(iterator);
    }
    
    assert_no_null(new_face->flout);
    assert_no_null(new_face->floops);
    
    return new_face;
}

// ------------------------------------------------------------------------------------------

void csmface_destruye(struct csmface_t **face)
{
    assert_no_null(face);
    assert_no_null(*face);

    if ((*face)->floops != NULL)
        csmnode_free_node_list(&(*face)->floops, csmloop_t);
    
    FREE_PP(face, struct csmface_t);
}

// ------------------------------------------------------------------------------------------

unsigned long csmface_id(const struct csmface_t *face)
{
    assert_no_null(face);
    return face->id;
}

// ------------------------------------------------------------------------------------------

struct csmsolid_t *csmface_fsolid(struct csmface_t *face)
{
    assert_no_null(face);
    return face->fsolid;
}

// ------------------------------------------------------------------------------------------

void csmface_set_fsolid(struct csmface_t *face, struct csmsolid_t *solid)
{
    assert_no_null(face);
    face->fsolid = solid;
}

// ------------------------------------------------------------------------------------------

struct csmloop_t *csmface_flout(struct csmface_t *face)
{
    assert_no_null(face);
    return face->flout;
}

// ------------------------------------------------------------------------------------------

void csmface_set_flout(struct csmface_t *face, struct csmloop_t *loop)
{
    assert_no_null(face);
    
    if (face->floops == NULL)
        face->floops = loop;
    
    face->flout = loop;
}

// ------------------------------------------------------------------------------------------

struct csmloop_t *csmface_floops(struct csmface_t *face)
{
    assert_no_null(face);
    return face->floops;
}

// ------------------------------------------------------------------------------------------

void csmface_set_floops(struct csmface_t *face, struct csmloop_t *loop)
{
    assert_no_null(face);
    face->floops = loop;
}

// ----------------------------------------------------------------------------------------------------

void csmface_add_loop_while_removing_from_old(struct csmface_t *face, struct csmloop_t *loop)
{
    struct csmface_t *loop_old_face;
    
    assert_no_null(face);
    assert_no_null(loop);
    
    loop_old_face = csmloop_lface(loop);
    assert_no_null(loop_old_face);
    assert_no_null(loop_old_face->floops);
    
    if (loop_old_face->floops == loop)
        loop_old_face->floops = csmloop_next(loop_old_face->floops);

    if (loop_old_face->flout == loop)
        loop_old_face->flout = loop_old_face->floops;
    
    csmloop_set_lface(loop, face);
    
    if (face->floops == NULL)
    {
        csmnode_remove_from_own_list(loop, csmloop_t);
        
        face->floops = loop;
        face->flout = loop;
    }
    else
    {
        csmnode_insert_node2_before_node1(face->floops, loop, csmloop_t);
        face->floops = loop;
    }
}

// ----------------------------------------------------------------------------------------------------

void csmface_remove_loop(struct csmface_t *face, struct csmloop_t **loop)
{
    assert_no_null(face);
    assert_no_null(loop);
    assert(face == csmloop_lface(*loop));
    
    if (face->floops == *loop)
        face->floops = csmloop_next(face->floops);

    if (face->flout == *loop)
        face->flout = face->floops;
    
    csmnode_free_node_in_list(loop, csmloop_t);
}
