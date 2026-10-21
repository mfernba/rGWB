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

#ifdef RGWB_STANDALONE_DISTRIBUTABLE

#include "csmassert.inl"
#include "csmmem.inl"

#else

#include "cyassert.h"
#include "cypespy.h"
#include "cypeio.h"
#include "cypetxt.h"
#include "standarc.h"

#define fopen lc_fopen
#define fopen_u lc_fopen_u
#define fread lc_fread
#define fclose lc_fclose
#define fprintf ctxt_printf
#define feof f_lc_feof

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

csmArrayStruct(i_stl_solid_t);

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

static void i_append_facet_to_brep_builder(const struct i_stl_facet_t *facet, struct csmfacbrep2solid_t *brep_builder)
{
    struct csmfacbrep2solid_loop_t *outer_loop;
    struct csmfacbrep2solid_face_t *face;
    
    assert_no_null(facet);

    outer_loop = csmfacbrep2solid_new_loop();
    csmfacbrep2solid_append_point_to_loop(outer_loop, (double)facet->v1x, (double)facet->v1y, (double)facet->v1z);
    csmfacbrep2solid_append_point_to_loop(outer_loop, (double)facet->v2x, (double)facet->v2y, (double)facet->v2z);
    csmfacbrep2solid_append_point_to_loop(outer_loop, (double)facet->v3x, (double)facet->v3y, (double)facet->v3z);
    
    face = csmfacbrep2solid_new_face();
    csmfacbrep2solid_append_outer_loop_to_face(face, &outer_loop);
    
    csmfacbrep2solid_append_face(brep_builder, &face);
}

// --------------------------------------------------------------------------------

static void i_append_face_to_stl_solid(struct i_stl_solid_t *stl_solid, const struct i_stl_facet_t *facet)
{
    assert_no_null(stl_solid);
    i_append_facet_to_brep_builder(facet, stl_solid->brep_builder);
}

// --------------------------------------------------------------------------------

static CSMBOOL i_did_evaluate_brep(
                        struct csmfacbrep2solid_t *brep_builder,
                        struct csmsolid_t **solid,
                        enum csmstlimporter_result_t *err_code)
{
    CSMBOOL did_evaluate;
    enum csmstlimporter_result_t err_code_loc;
    enum csmfacbrep2solid_result_t brepbuilder_result;
    
    assert_no_null(err_code);
    
    brepbuilder_result = csmfacbrep2solid_build(brep_builder, solid);

    switch (brepbuilder_result)
    {
        case CSMFACBREP2SOLID_RESULT_OK:
        case CSMFACBREP2SOLID_RESULT_EMPTY_SOLID:
            
            did_evaluate = CSMTRUE;
            err_code_loc = CSMSTLIMPORTER_RESULT_OK;
            break;
            
        case CSMFACBREP2SOLID_RESULT_NON_MANIFOLD_FACETED_BREP:
            
            did_evaluate = CSMFALSE;
            err_code_loc = CSMSTLIMPORTER_RESULT_NON_MANIFOLD_FACETED_BREP;
            break;
            
        case CSMFACBREP2SOLID_RESULT_MALFORMED_FACETED_BREP:
            
            did_evaluate = CSMFALSE;
            err_code_loc = CSMSTLIMPORTER_RESULT_MALFORMED_FACETED_BREP;
            break;
            
        case CSMFACBREP2SOLID_RESULT_INCONSISTENT_INNER_LOOP_ORIENTATION:
            
            did_evaluate = CSMFALSE;
            err_code_loc = CSMSTLIMPORTER_RESULT_INCONSISTENT_INNER_LOOP_ORIENTATION;
            break;
            
        default_error();
    }
    
    *err_code = err_code_loc;
    
    return did_evaluate;
}

// --------------------------------------------------------------------------------

static enum csmstlimporter_result_t i_did_evaluate_optree(struct csmoptree_t *optree, struct csmsolid_t **solid)
{
    enum csmoptree_result_t optree_result;
    
    optree_result = csmoptree_evaluate(optree, solid);
    
    switch (optree_result)
    {
        case CSMOPTREE_RESULT_OK:
            
            return CSMSTLIMPORTER_RESULT_OK;
            
        case CSMOPTREE_RESULT_SETOP_NON_MANIFOLD:
            
            return CSMSTLIMPORTER_RESULT_NON_MANIFOLD_FACETED_BREP;
            
        case CSMOPTREE_RESULT_SETOP_IMPROPER_INTERSECTIONS:
            
            return CSMSTLIMPORTER_RESULT_INCONSISTENT_IMPROPER_INTERSECTIONS;
            
        case CSMOPTREE_RESULT_SPLIT_NO_SPLIT:
        case CSMOPTREE_RESULT_SPLIT_IMPROPER_CUT:
        default_error();
    }
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
        struct csmsolid_t *solid_i;
        
        stl_solid = csmarrayc_get_st(stl_solids, i, i_stl_solid_t);
        assert_no_null(stl_solid);
        
        if (i_did_evaluate_brep(stl_solid->brep_builder, &solid_i, &result) == CSMFALSE)
        {
            exists_error = CSMTRUE;
        }
        else
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
        }
        
        if (exists_error == CSMTRUE)
            break;
    }
    
    if (exists_error == CSMTRUE)
        solid_loc = NULL;
    else
        result = i_did_evaluate_optree(optree, &solid_loc);
        
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
    
    if (fread(&header, sizeof(struct i_stl_header_t), 1, file) != 1 || header.no_facets == 0)
    {
        result = CSMSTLIMPORTER_RESULT_INCORRECT_FORMAT;
        solid_loc = NULL;
    }
    else
    {
        csmArrayStruct(i_stl_solid_t) *stl_solids;
        struct i_stl_solid_t *current_stl_solid;
        CSMBOOL is_truncated;
        unsigned long i;

        stl_solids = csmarrayc_new_st_array(0, i_stl_solid_t);
        current_stl_solid = NULL;
        is_truncated = CSMFALSE;

        for (i = 0; i < header.no_facets; i++)
        {
            struct i_stl_facet_t facet;
            unsigned short attrib;

            // File shorter than the facet count in header...
            if (fread(&facet, sizeof(struct i_stl_facet_t), 1, file) != 1
                    || fread(&attrib, sizeof(unsigned short), 1, file) != 1)
            {
                is_truncated = CSMTRUE;
                break;
            }

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

        if (is_truncated == CSMTRUE)
        {
            result = CSMSTLIMPORTER_RESULT_INCORRECT_FORMAT;
            solid_loc = NULL;
        }
        else
        {
            result = i_new_solid_from_stl_solids(stl_solids, &solid_loc);
        }

        csmarrayc_free_st(&stl_solids, i_stl_solid_t, i_free_stl_solid);
    }
    
    *solid = solid_loc;
    
    return result;
}

#ifdef RGWB_STANDALONE_DISTRIBUTABLE

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

// --------------------------------------------------------------------------------

static CSMBOOL i_did_read_ascii_facet_loop(FILE *file, struct i_stl_facet_t *facet)
{
    int outer_loop_length, endloop_length, endfacet_length;

    assert_no_null(facet);

    // fscanf returns 0 with or without match on literal-only formats, %n tells if it matched...
    outer_loop_length = -1;
    fscanf(file, " outer loop%n", &outer_loop_length);

    if (outer_loop_length == -1)
        return CSMFALSE;

    if (fscanf(file, " vertex %f %f %f", &facet->v1x, &facet->v1y, &facet->v1z) != 3)
        return CSMFALSE;

    if (fscanf(file, " vertex %f %f %f", &facet->v2x, &facet->v2y, &facet->v2z) != 3)
        return CSMFALSE;

    if (fscanf(file, " vertex %f %f %f", &facet->v3x, &facet->v3y, &facet->v3z) != 3)
        return CSMFALSE;

    endloop_length = -1;
    fscanf(file, " endloop%n", &endloop_length);

    if (endloop_length == -1)
        return CSMFALSE;

    endfacet_length = -1;
    fscanf(file, " endfacet%n", &endfacet_length);

    if (endfacet_length == -1)
        return CSMFALSE;

    return CSMTRUE;
}

// --------------------------------------------------------------------------------

static enum csmstlimporter_result_t i_did_read_ascii_file(FILE *file, struct csmsolid_t **solid)
{
    enum csmstlimporter_result_t result;
    struct csmsolid_t *solid_loc;
    unsigned long no_facets;
    struct csmoptree_t *optree;
    CSMBOOL exists_error;
    
    assert_no_null(solid);
    
    result = CSMSTLIMPORTER_RESULT_EMPTY_SOLID;
    solid_loc = NULL;
    no_facets = 0;
    
    optree = NULL;
    exists_error = CSMFALSE;
    
    do
    {
        int solid_header_length, solid_footer_length;
        unsigned long no_facets_solid;
        CSMBOOL facet_format_error;
        struct i_stl_facet_t facet;
        struct csmfacbrep2solid_t *brep_builder;
        struct csmsolid_t *solid_i;

        // Trailing whitespace after the last solid isn't another solid...
        fscanf(file, " ");

        if (feof(file))
            break;

        // Solid name is optional and may contain spaces, skip the whole line...
        solid_header_length = -1;
        fscanf(file, " solid%n%*[^\n]", &solid_header_length);

        brep_builder = csmfacbrep2solid_new(1.e-9, CSMTRUE);
        no_facets_solid = 0;

        facet_format_error = CSMFALSE;

        while (fscanf(file, " facet normal %f %f %f\n", &facet.nx, &facet.ny, &facet.nz) == 3)
        {
            no_facets++;
            no_facets_solid++;

            if (i_did_read_ascii_facet_loop(file, &facet) == CSMFALSE)
            {
                facet_format_error = CSMTRUE;
                break;
            }

            i_append_facet_to_brep_builder(&facet, brep_builder);
        }

        solid_footer_length = -1;

        if (facet_format_error == CSMFALSE)
            fscanf(file, " endsolid%n%*[^\n]", &solid_footer_length);

        if (facet_format_error == CSMTRUE)
        {
            exists_error = CSMTRUE;
            result = CSMSTLIMPORTER_RESULT_INCORRECT_FORMAT;
        }
        else if (solid_header_length == -1 && no_facets_solid == 0 && solid_footer_length == -1)
        {
            // Nothing recognized in this pass: the file would never advance...
            exists_error = CSMTRUE;
            result = CSMSTLIMPORTER_RESULT_INCORRECT_FORMAT;
        }
        else if (i_did_evaluate_brep(brep_builder, &solid_i, &result) == CSMFALSE)
        {
            exists_error = CSMTRUE;
        }
        else if (solid_i != NULL)
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
        }

        csmfacbrep2solid_free(&brep_builder);
        
    } while (!feof(file) && exists_error == CSMFALSE);
    
    if (exists_error == CSMTRUE)
    {
        solid_loc = NULL;
    }
    else if (optree == NULL)
    {
        result = CSMSTLIMPORTER_RESULT_EMPTY_SOLID;
        solid_loc = NULL;
    }
    else
    {
        result = i_did_evaluate_optree(optree, &solid_loc);
    }

    *solid = solid_loc;

    if (optree != NULL)
        csmoptree_free(&optree);

    return result;
}

// --------------------------------------------------------------------------------

enum csmstlimporter_result_t csmstlimporter_did_read_ascii_stl(const char *file_path, struct csmsolid_t **solid)
{
    enum csmstlimporter_result_t result;
    struct csmsolid_t *solid_loc;
    FILE *file;
    
    assert_no_null(solid);
    
    file = fopen(file_path, "rt");
    
    if (file == NULL)
    {
        result = CSMSTLIMPORTER_RESULT_FILE_NOT_FOUND;
        solid_loc = NULL;
    }
    else
    {
        result = i_did_read_ascii_file(file, &solid_loc);
        fclose(file);
    }
    
    *solid = solid_loc;
    
    return result;
}

#else

// --------------------------------------------------------------------------------

enum csmstlimporter_result_t csmstlimporter_did_read_binary_stl_u(const wchar_t *file_path, struct csmsolid_t **solid)
{
    enum csmstlimporter_result_t result;
    struct csmsolid_t *solid_loc;
    FILE *file;
    
    assert_no_null(solid);
    
    file = fopen_u(file_path, L"rb");
    
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

// --------------------------------------------------------------------------------

void csmstlimporter_convert_binary_stl_to_text_u(const wchar_t *file_path_in, const wchar_t *file_path_out)
{
    FILE *file_in, *file_out;
    size_t stl_facet_size;
    struct i_stl_header_t header;
    unsigned long i;

    assert(sizeof(unsigned int) == 4);
    assert(sizeof(float) == 4);
    assert(sizeof(unsigned short) == 2);
    stl_facet_size = sizeof(struct i_stl_facet_t);
    assert(stl_facet_size == 48);

    file_in = fopen_u(file_path_in, L"rb");
    file_out = fopen_u(file_path_out, L"wt");
    
    fread(&header, sizeof(struct i_stl_header_t), 1, file_in);
    assert(header.no_facets > 0);

    fprintf(file_out, "%s\n", header.header);
    fprintf(file_out, "%lu\n", header.no_facets);
    fprintf(file_out, "\n");
    
    for (i = 0; i < header.no_facets; i++)
    {
        struct i_stl_facet_t facet;
        unsigned short attrib;
            
        fread(&facet, sizeof(struct i_stl_facet_t), 1, file_in);
        fread(&attrib, sizeof(unsigned short), 1, file_in);

        fprintf(file_out, "Face idx: %lu\n", i);
        fprintf(file_out, "Normal %f %f %f\n", facet.nx, facet.ny, facet.nz);
        fprintf(file_out, "Vertex %f %f %f\n", facet.v1x, facet.v1y, facet.v1z);
        fprintf(file_out, "Vertex %f %f %f\n", facet.v2x, facet.v2y, facet.v2z);
        fprintf(file_out, "Vertex %f %f %f\n", facet.v3x, facet.v3y, facet.v3z);
        fprintf(file_out, "\n");
    }

    fclose(file_in);
    fclose(file_out);
}

#endif
