//
//  csmstlimporter.c
//  rGWB
//
//  Created by Manuel Fernandez on 29/10/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmstlimporter.h"
#include "csmstlimporter.hxx"

#include "csmarrayc.h"
#include "csmfacbrep2solid.h"
#include "csmoptree.h"
#include "csmoptree.hxx"

#ifdef __STANDALONE_DISTRIBUTABLE

#include "csmassert.inl"
#include "csmmem.inl"

#else

#include "cyassert.h"
#include "cypespy.h"

#endif

struct i_stl_header_t
{
    char header[80];
    unsigned int no_facets;
};

struct i_stl_facet_t
{
    float nx, ny, nz;
    
    float v1x, v1y, v1z;
    float v2x, v2y, v2z;
    float v3x, v3y, v3z;
};

struct i_stl_solid_t
{
    unsigned short attrib;
    struct csmfacbrep2solid_t *brep_builder;
};

csmArrayStruct(i_stl_solid_t) *stl_solids;

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_stl_solid_t *, i_new_stl_solid, (
                        unsigned short attrib,
                        struct csmfacbrep2solid_t **brep_builder))
{
    struct i_stl_solid_t *stl_solid;
    
    stl_solid = MALLOC(struct i_stl_solid_t);
    
    stl_solid->attrib = attrib;
    stl_solid->brep_builder = ASSIGN_POINTER_PP_NOT_NULL(brep_builder, struct csmfacbrep2solid_t);
    
    return stl_solid;
}

// --------------------------------------------------------------------------------

static void i_free_stl_solid(struct i_stl_solid_t **stl_solid)
{
    assert_no_null(stl_solid);
    assert_no_null(*stl_solid);
    
    csmfacbrep2solid_free(&(*stl_solid)->brep_builder);
    
    FREE_PP(stl_solid, struct i_stl_solid_t);
}

// --------------------------------------------------------------------------------

static CSMBOOL i_is_stl_solid_with_attrib(const struct i_stl_solid_t *stl_solid, const unsigned short *attrib)
{
    assert_no_null(stl_solid);
    assert_no_null(attrib);
    
    return IS_TRUE(stl_solid->attrib == *attrib);
}

// --------------------------------------------------------------------------------

static void i_append_face_to_stl_solid(struct i_stl_solid_t *stl_solid, const struct i_stl_facet_t *facet)
{
    struct csmfacbrep2solid_loop_t *outer_loop;
    struct csmfacbrep2solid_face_t *face;
    
    assert_no_null(stl_solid);
    assert_no_null(facet);

    outer_loop = csmfacbrep2solid_new_loop();
    csmfacbrep2solid_append_point_to_loop(outer_loop, facet->v1x, facet->v1y, facet->v1z);
    csmfacbrep2solid_append_point_to_loop(outer_loop, facet->v2x, facet->v2y, facet->v2z);
    csmfacbrep2solid_append_point_to_loop(outer_loop, facet->v3x, facet->v3y, facet->v3z);
    
    face = csmfacbrep2solid_new_face();
    csmfacbrep2solid_append_outer_loop_to_face(face, &outer_loop);
    
    csmfacbrep2solid_append_face(stl_solid->brep_builder, &face);
}

// --------------------------------------------------------------------------------

static enum csmstlimporter_result_t i_new_solid_from_stl_solids(csmArrayStruct(i_stl_solid_t) *stl_solids, struct csmsolid_t **solid)
{
    enum csmstlimporter_result_t result;
    struct csmsolid_t *solid_loc;
    unsigned long i, no_stl_solids;
    struct csmoptree_t *optree;
    CSMBOOL exists_error;
    
    no_stl_solids = csmarrayc_count_st(stl_solids, i_stl_solid_t);
    assert(no_stl_solids > 0);
    
    result = CSMSTLIMPORTER_RESULT_EMPTY_SOLID;
    exists_error = CSMFALSE;
    optree = NULL;
    
    for (i = 0; i < no_stl_solids; i++)
    {
        struct i_stl_solid_t *stl_solid;
        enum csmfacbrep2solid_result_t brepbuilder_result;
        struct csmsolid_t *solid_i;
        
        stl_solid = csmarrayc_get_st(stl_solids, i, i_stl_solid_t);
        assert_no_null(stl_solid);
        
        brepbuilder_result = csmfacbrep2solid_build(stl_solid->brep_builder, &solid_i);
        
        switch (brepbuilder_result)
        {
            case CSMFACBREP2SOLID_RESULT_OK:
            {
                if (optree == NULL)
                {
                    optree = csmoptree_new_node_solid(&solid_i);
                }
                else
                {
                    struct csmoptree_t *optree_i;
                    
                    optree_i = csmoptree_new_node_solid(&solid_i);
                    optree = csmoptree_new_node_boolean_union(&optree, &optree_i);
                }
                break;
            }
                
            case CSMFACBREP2SOLID_RESULT_EMPTY_SOLID:
                break;
                
            case CSMFACBREP2SOLID_RESULT_NON_MANIFOLD_FACETED_BREP:
                
                exists_error = CSMTRUE;
                result = CSMSTLIMPORTER_RESULT_NON_MANIFOLD_FACETED_BREP;
                break;
                
            case CSMFACBREP2SOLID_RESULT_MALFORMED_FACETED_BREP:
                
                exists_error = CSMTRUE;
                result = CSMSTLIMPORTER_RESULT_MALFORMED_FACETED_BREP;
                break;
                
            case CSMFACBREP2SOLID_RESULT_INCONSISTENT_INNER_LOOP_ORIENTATION:
                
                exists_error = CSMTRUE;
                result = CSMSTLIMPORTER_RESULT_INCONSISTENT_INNER_LOOP_ORIENTATION;
                break;
                
            default_error();
        }
        
        if (exists_error == CSMTRUE)
            break;
    }
    
    if (exists_error == CSMTRUE)
    {
        solid_loc = NULL;
    }
    else
    {
        enum csmoptree_result_t optree_result;
        
        optree_result = csmoptree_evaluate(optree, &solid_loc);
        
        switch (optree_result)
        {
            case CSMOPTREE_RESULT_OK:
                
                result = CSMSTLIMPORTER_RESULT_OK;
                break;
                
            case CSMOPTREE_RESULT_SETOP_NON_MANIFOLD:
                
                result = CSMSTLIMPORTER_RESULT_NON_MANIFOLD_FACETED_BREP;
                break;
                
            case CSMOPTREE_RESULT_SETOP_IMPROPER_INTERSECTIONS:
                
                result = CSMSTLIMPORTER_RESULT_INCONSISTENT_IMPROPER_INTERSECTIONS;
                break;
                
            case CSMOPTREE_RESULT_SPLIT_NO_SPLIT:
            case CSMOPTREE_RESULT_SPLIT_IMPROPER_CUT:
            default_error();
        }
    }
            
    *solid = solid_loc;
    
    if (optree != NULL)
        csmoptree_free(&optree);
    
    return result;
}

// --------------------------------------------------------------------------------

static enum csmstlimporter_result_t i_did_read_binary_file(FILE *file, struct csmsolid_t **solid)
{
    enum csmstlimporter_result_t result;
    size_t stl_facet_size;
    struct csmsolid_t *solid_loc;
    struct i_stl_header_t header;
    
    assert_no_null(solid);
    assert(sizeof(unsigned int) == 4);
    assert(sizeof(float) == 4);
    assert(sizeof(unsigned short) == 2);
    
    stl_facet_size = sizeof(struct i_stl_facet_t);
    assert(stl_facet_size == 48);
    
    fread(&header, sizeof(struct i_stl_header_t), 1, file);
    assert(header.no_facets > 0);
    
    if (header.no_facets == 0)
    {
        result = CSMSTLIMPORTER_RESULT_INCORRECT_FORMAT;
        solid_loc = NULL;
    }
    else
    {
        csmArrayStruct(i_stl_solid_t) *stl_solids;
        struct i_stl_solid_t *current_stl_solid;
        unsigned long i;
    
        stl_solids = csmarrayc_new_st_array(0, i_stl_solid_t);
        current_stl_solid = NULL;
        
        for (i = 0; i < header.no_facets; i++)
        {
            struct i_stl_facet_t facet;
            unsigned short attrib;
            
            fread(&facet, sizeof(struct i_stl_facet_t), 1, file);
            fread(&attrib, sizeof(unsigned short), 1, file);
            
            if (current_stl_solid == NULL || i_is_stl_solid_with_attrib(current_stl_solid, &attrib) == CSMFALSE)
            {
                unsigned long pos;
                
                if (csmarrayc_contains_element_st(stl_solids, i_stl_solid_t, &attrib, unsigned short, i_is_stl_solid_with_attrib, &pos) == CSMTRUE)
                {
                    current_stl_solid = csmarrayc_get_st(stl_solids, pos, i_stl_solid_t);
                }
                else
                {
                    struct csmfacbrep2solid_t *brep_builder;
                    
                    brep_builder = csmfacbrep2solid_new(1.e-9, CSMTRUE);
                    current_stl_solid = i_new_stl_solid(attrib, &brep_builder);
                    
                    csmarrayc_append_element_st(stl_solids, current_stl_solid, i_stl_solid_t);
                }
            }

            i_append_face_to_stl_solid(current_stl_solid, &facet);
        }
        
        result = i_new_solid_from_stl_solids(stl_solids, &solid_loc);
        csmarrayc_free_st(&stl_solids, i_stl_solid_t, i_free_stl_solid);
    }
    
    *solid = solid_loc;
    
    return result;
}

// --------------------------------------------------------------------------------

enum csmstlimporter_result_t csmstlimporter_did_read_binary_stl(const char *file_path, struct csmsolid_t **solid)
{
    enum csmstlimporter_result_t result;
    struct csmsolid_t *solid_loc;
    FILE *file;
    
    assert_no_null(solid);
    
    file = fopen(file_path, "rb");
    
    if (file == NULL)
    {
        result = CSMSTLIMPORTER_RESULT_FILE_NOT_FOUND;
        solid_loc = NULL;
    }
    else
    {
        result = i_did_read_binary_file(file, &solid_loc);
        fclose(file);
    }
    
    *solid = solid_loc;
    
    return result;
}
