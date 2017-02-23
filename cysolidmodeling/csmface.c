// Face...

#include "csmface.inl"

#include "csmbbox.inl"
#include "csmnode.inl"

#include "cyassert.h"
#include "cypeid.h"
#include "cypespy.h"

struct csmface_t
{
    struct csmnode_t clase_base;

    struct csmsolid_t *fsolid;
    
    struct csmloop_t *flout;
    struct csmloop_t *floops;
    
    double A, B, C, D;
    struct csmbbox_t *bbox;
};

// ------------------------------------------------------------------------------------------

static void i_csmface_destruye(struct csmface_t **face)
{
    assert_no_null(face);
    assert_no_null(*face);

    csmnode_release_ex((*face)->fsolid, csmsolid_t);
    
    csmnode_release_ex((*face)->flout, csmloop_t);
    csmnode_release_ex((*face)->floops, csmloop_t);
    
    FREE_PP(face, struct csmface_t);
}

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
    
    face->clase_base = csmnode_crea_node(id, face, i_csmface_destruye, csmface_t);
    
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

struct csmface_t *csmface_crea(struct csmsolid_t *fsolid, unsigned long *id_nuevo_elemento)
{
    unsigned long id;
    struct csmsolid_t *fsolid_loc;
    struct csmloop_t *flout;
    struct csmloop_t *floops;
    double A, B, C, D;
    struct csmbbox_t *bbox;
    
    id = cypeid_nuevo_id(id_nuevo_elemento, NULL);

    fsolid_loc = csmnode_retain_ex(fsolid, csmsolid_t);
    flout = NULL;
    floops = NULL;
    
    A = 0.;
    B = 0.;
    C = 0.;
    D = 0.;
    
    bbox = csmbbox_crea_vacia();
    
    return i_crea(id, fsolid_loc, flout, floops, A, B, C, D, &bbox);
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
    
    csmnode_release_ex(face->fsolid, csmsolid_t);
    face->fsolid = csmnode_retain_ex(solid, csmsolid_t);
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
    
    csmnode_release_ex(face->flout, csmloop_t);
    face->flout = csmnode_retain_ex(loop, csmloop_t);
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
    
    csmnode_release_ex(face->floops, csmloop_t);
    face->floops = csmnode_retain_ex(loop, csmloop_t);
}

// ----------------------------------------------------------------------------------------------------

struct csmface_t *csmface_next(struct csmface_t *face)
{
    assert_no_null(face);
    return csmnode_downcast(csmnode_next(CSMNODE(face)), csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmface_set_next(struct csmface_t *face, struct csmface_t *next_face)
{
    assert_no_null(face);
    assert_no_null(next_face);
    
    csmnode_set_ptr_next(CSMNODE(face), CSMNODE(next_face));
}

// ----------------------------------------------------------------------------------------------------

struct csmface_t *csmface_prev(struct csmface_t *face)
{
    assert_no_null(face);
    return csmnode_downcast(csmnode_prev(CSMNODE(face)), csmface_t);
}

// ----------------------------------------------------------------------------------------------------

void csmface_set_prev(struct csmface_t *face, struct csmface_t *prev_face)
{
    assert_no_null(face);
    assert_no_null(prev_face);

    csmnode_set_ptr_prev(CSMNODE(face), CSMNODE(prev_face));
}

