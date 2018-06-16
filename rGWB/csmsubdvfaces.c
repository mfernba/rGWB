// Subdivides non planar faces into planar components...

#include "csmsubdvfaces.inl"

#include "csmdebug.inl"
#include "csmeuler_lmef.inl"
#include "csmface.inl"
#include "csmhashtb.inl"
#include "csmhedge.inl"
#include "csmloop.inl"
#include "csmmath.inl"
#include "csmsolid.inl"
#include "csmvertex.inl"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

// ------------------------------------------------------------------------------------------

static CSMBOOL i_did_find_3_non_collinear_vertexs_in_loop(
                        struct csmloop_t *loop,
                        struct csmvertex_t **v1, struct csmvertex_t **v2, struct csmvertex_t **v3)
{
    CSMBOOL did_find_3_non_collinear_vertexs;
    struct csmhedge_t *ledge, *he_iterator;
    struct csmvertex_t *v1_loc, *v2_loc, *v3_loc;
    
    assert_no_null(v1);
    assert_no_null(v2);
    assert_no_null(v3);
    
    ledge = csmloop_ledge(loop);
    he_iterator = ledge;
    
    did_find_3_non_collinear_vertexs = CSMFALSE;
    v1_loc = NULL;
    v2_loc = NULL;
    v3_loc = NULL;
    
    do
    {
        struct csmvertex_t *vertex;
        
        vertex = csmhedge_vertex(he_iterator);
        
        if (v1_loc == NULL)
        {
            v1_loc = vertex;
        }
        else if (v2_loc == NULL)
        {
            v2_loc = vertex;
        }
        else
        {
            double Ux1, Uy1, Uz1, Ux2, Uy2, Uz2;
            double Wx, Wy, Wz;
            
            csmvertex_vector_from_vertex1_to_vertex2(v1_loc, v2_loc, &Ux1, &Uy1, &Uz1);
            csmvertex_vector_from_vertex1_to_vertex2(v1_loc, vertex, &Ux2, &Uy2, &Uz2);
            
            csmmath_cross_product3D(Ux1, Uy1, Uz1, Ux2, Uy2, Uz2, &Wx, &Wy, &Wz);
            
            if (csmmath_is_null_vector(Wx, Wy, Wz, 1.e-10) == CSMFALSE)
            {
                did_find_3_non_collinear_vertexs = CSMTRUE;
                v3_loc = vertex;
                break;
            }
        }
        
        he_iterator = csmhedge_next(he_iterator);
        
    } while (he_iterator != ledge);
    
    *v1 = v1_loc;
    *v2 = v2_loc;
    *v3 = v3_loc;
    
    return did_find_3_non_collinear_vertexs;
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_did_subdivide_face(struct csmface_t *face)
{
    CSMBOOL did_subdivide_face;
    struct csmloop_t *flout;
    struct csmvertex_t *v1, *v2, *v3;
    
    assert(csmface_has_holes(face) == CSMFALSE);

    flout = csmface_flout(face);
    
    if (i_did_find_3_non_collinear_vertexs_in_loop(flout, &v1, &v2, &v3) == CSMFALSE)
    {
        did_subdivide_face = CSMFALSE;
    }
    else
    {
        double A, B, C, D;
        struct csmhedge_t *ledge, *he_iterator;
        
        did_subdivide_face = CSMFALSE;
        csmvertex_implicit_plane_equation_given_3_vertexs(v1, v2, v3, &A, &B, &C, &D);
        
        ledge = csmloop_ledge(flout);
        he_iterator = ledge;
        
        do
        {
            struct csmvertex_t *vertex;
            double distance_to_plane;
            
            vertex = csmhedge_vertex(he_iterator);
            distance_to_plane = csmvertex_signed_distance_to_plane(vertex, A, B, C, D);
            
            if (csmmath_compare_doubles(distance_to_plane, 0., 1.e-6) != CSMCOMPARE_EQUAL)
            {
                struct csmface_t *new_face;
                
                assert(vertex != v1 && vertex != v2 && vertex != v3);
                
                did_subdivide_face = CSMTRUE;
                
                csmeuler_lmef(ledge, csmhedge_prev(he_iterator), &new_face, NULL, NULL);
                
                if (csmdebug_debug_enabled() == CSMTRUE)
                {
                    csmface_print_info_debug(face, CSMTRUE, NULL);
                    csmface_print_info_debug(new_face, CSMTRUE, NULL);
                }
            }
            
            if (did_subdivide_face == CSMTRUE)
                break;
            else
                he_iterator = csmhedge_next(he_iterator);
            
        } while (he_iterator != ledge);
    }
    
    return did_subdivide_face;
}

// ------------------------------------------------------------------------------------------

void csmsubdvfaces_subdivide_faces(struct csmsolid_t *solid)
{
    CSMBOOL faces_subdivided;
    CSMBOOL did_modify_solid;
    
    assert_no_null(solid);
    
    did_modify_solid = CSMFALSE;
    
    do
    {
        struct csmhashtb_iterator(csmface_t) *face_iterator;
        
        //if (csmdebug_debug_enabled() == CSMTRUE)
            //csmsolid_print_debug(solid, CSMTRUE);

        face_iterator = csmsolid_face_iterator(solid);
        faces_subdivided = CSMFALSE;
    
        while (csmhashtb_has_next(face_iterator, csmface_t) == CSMTRUE)
        {
            struct csmface_t *face;
            
            csmhashtb_next_pair(face_iterator, NULL, &face, csmface_t);
            
            if (i_did_subdivide_face(face) == CSMTRUE)
            {
                faces_subdivided = CSMTRUE;
                did_modify_solid = CSMTRUE;
                break;
            }
        }

        csmhashtb_free_iterator(&face_iterator, csmface_t);
        
    } while (faces_subdivided == CSMTRUE);
    
    if (did_modify_solid == CSMTRUE)
        csmsolid_redo_geometric_face_data(solid);
}

