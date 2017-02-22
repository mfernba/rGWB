// Face...

#include "csmface.inl"

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
    
    face->clase_base = csmnode_crea_node(id, face, csmface_destruye, csmface_t);
    
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

struct csmface_t *csmface_crea(unsigned long *id_nuevo_elemento)
{
}
