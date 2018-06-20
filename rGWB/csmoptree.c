//
//  csmoptree.c
//  rGWB
//
//  Created by Manuel Fernández on 15/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmoptree.h"
#include "csmoptree.hxx"

#include "csmid.inl"
#include "csmsetop.h"
#include "csmsetop.hxx"
#include "csmsolid.h"
#include "csmsplit.h"
#include "csmsplit.hxx"
#include "csmsave.h"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmmem.inl"
#include "csmassert.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

enum i_type_t
{
    i_TYPE_SOLID,
    
    i_TYPE_OPERATION_SETOP_UNION,
    i_TYPE_OPERATION_SETOP_DIFFERENCE,
    i_TYPE_OPERATION_SETOP_INTERSECTION,

    i_TYPE_OPERATION_SPLIT_ABOVE,
    i_TYPE_OPERATION_SPLIT_BELOW,

    i_TYPE_OPERATION_GENERAL_TRANSFORM
};

struct i_setop_params_t
{
    struct csmoptree_t *node1;
    struct csmoptree_t *node2;
};

struct i_transform_t
{
    double Ux, Uy, Uz, Dx;
    double Vx, Vy, Vz, Dy;
    double Wx, Wy, Wz, Dz;
    
    struct csmoptree_t *node;
};

struct i_split_plane_t
{
    double A, B, C, D;
    struct csmoptree_t *node;
};

union operand_t
{
    struct csmsolid_t *solid;
    struct i_setop_params_t setop_operation;
    struct i_transform_t transform;
    struct i_split_plane_t split_plane;
};

struct csmoptree_t
{
    unsigned long node_id;
    
    enum i_type_t type;
    union operand_t operands;
    
    enum csmoptree_result_t evaluation_result;
    struct csmsolid_t *solid_result;
};

static const unsigned char i_FILE_VERSION = 0;

static const enum csmoptree_result_t i_UNEVALUATED_RESULT = (enum csmoptree_result_t)9999;

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmoptree_t *, i_new, (unsigned long node_id, enum i_type_t type, enum csmoptree_result_t evaluation_result, struct csmsolid_t **solid_result))
{
    struct csmoptree_t *optree;
    
    optree = MALLOC(struct csmoptree_t);
    
    optree->node_id = node_id;
    
    optree->type = type;
    
    optree->evaluation_result = evaluation_result;
    optree->solid_result = ASSIGN_POINTER_PP(solid_result, struct csmsolid_t);
    
    return optree;
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmoptree_t *, i_new_unevaluated_optree, (enum i_type_t type))
{
    unsigned long node_id;
    struct csmsolid_t *solid_result;
    enum csmoptree_result_t evaluation_result;
    
    node_id = 0;
    solid_result = NULL;
    evaluation_result = i_UNEVALUATED_RESULT;
    
    return i_new(node_id, type, evaluation_result, &solid_result);
}

// ------------------------------------------------------------------------------------------

void csmoptree_free(struct csmoptree_t **optree)
{
    assert_no_null(optree);
    assert_no_null(*optree);
    
    switch ((*optree)->type)
    {
        case i_TYPE_SOLID:
            
            csmsolid_free(&((*optree)->operands.solid));
            break;
        
        case i_TYPE_OPERATION_SETOP_UNION:
        case i_TYPE_OPERATION_SETOP_DIFFERENCE:
        case i_TYPE_OPERATION_SETOP_INTERSECTION:
            
            csmoptree_free(&((*optree)->operands.setop_operation.node1));
            csmoptree_free(&((*optree)->operands.setop_operation.node2));
            break;

        case i_TYPE_OPERATION_SPLIT_ABOVE:
        case i_TYPE_OPERATION_SPLIT_BELOW:
            
            csmoptree_free(&((*optree)->operands.split_plane.node));
            break;
            
        case i_TYPE_OPERATION_GENERAL_TRANSFORM:
            
            csmoptree_free(&((*optree)->operands.transform.node));
            break;
            
        default_error();
    }
    
    if ((*optree)->solid_result != NULL)
        csmsolid_free(&(*optree)->solid_result);
    
    FREE_PP(optree, struct csmoptree_t);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmoptree_t *, i_read_optree, (struct csmsave_t *csmsave))
{
    struct csmoptree_t *optree;
    unsigned long node_id;
    enum i_type_t type;
    
    node_id = csmsave_read_ulong(csmsave);
    type = csmsave_read_enum(csmsave, i_type_t);
    
    optree = i_new_unevaluated_optree(type);
    assert_no_null(optree);
    
    optree->node_id = node_id;
    
    switch (optree->type)
    {
        case i_TYPE_SOLID:
        {
            optree->operands.solid = csmsolid_read(csmsave);
            break;
        }
        
        case i_TYPE_OPERATION_SETOP_UNION:
        case i_TYPE_OPERATION_SETOP_DIFFERENCE:
        case i_TYPE_OPERATION_SETOP_INTERSECTION:
        {
            optree->operands.setop_operation.node1 = i_read_optree(csmsave);
            optree->operands.setop_operation.node2 = i_read_optree(csmsave);
            break;
        }

        case i_TYPE_OPERATION_SPLIT_ABOVE:
        case i_TYPE_OPERATION_SPLIT_BELOW:
        {
            optree->operands.split_plane.A = csmsave_read_double(csmsave);
            optree->operands.split_plane.B = csmsave_read_double(csmsave);
            optree->operands.split_plane.C = csmsave_read_double(csmsave);
            optree->operands.split_plane.D = csmsave_read_double(csmsave);
            
            optree->operands.split_plane.node = i_read_optree(csmsave);
            break;
        }
            
        case i_TYPE_OPERATION_GENERAL_TRANSFORM:
        {
            optree->operands.transform.Ux = csmsave_read_double(csmsave);
            optree->operands.transform.Uy = csmsave_read_double(csmsave);
            optree->operands.transform.Uz = csmsave_read_double(csmsave);
            optree->operands.transform.Dx = csmsave_read_double(csmsave);
            
            optree->operands.transform.Vx = csmsave_read_double(csmsave);
            optree->operands.transform.Vy = csmsave_read_double(csmsave);
            optree->operands.transform.Vz = csmsave_read_double(csmsave);
            optree->operands.transform.Dy = csmsave_read_double(csmsave);
            
            optree->operands.transform.Wx = csmsave_read_double(csmsave);
            optree->operands.transform.Wy = csmsave_read_double(csmsave);
            optree->operands.transform.Wz = csmsave_read_double(csmsave);
            optree->operands.transform.Dz = csmsave_read_double(csmsave);
            
            optree->operands.transform.node = i_read_optree(csmsave);
            break;
        }
            
        default_error();
    }
    
    optree->evaluation_result = csmsave_read_enum(csmsave, csmoptree_result_t);
    optree->solid_result = csmsave_read_optional_st(csmsave, csmsolid_read, csmsolid_t);
    
    return optree;
}

// ------------------------------------------------------------------------------------------

struct csmoptree_t *csmoptree_read(struct csmsave_t *csmsave)
{
    unsigned char file_version;
    
    file_version = csmsave_read_uchar(csmsave);
    assert(file_version == 0);
    
    return i_read_optree(csmsave);
}

// ------------------------------------------------------------------------------------------

static void i_write_optree(const struct csmoptree_t *optree, struct csmsave_t *csmsave)
{
    assert_no_null(optree);
    
    csmsave_write_ulong(csmsave, optree->node_id);
    csmsave_write_enum(csmsave, optree->type);
    
    switch (optree->type)
    {
        case i_TYPE_SOLID:
        {
            csmsolid_write(optree->operands.solid, csmsave);
            break;
        }
        
        case i_TYPE_OPERATION_SETOP_UNION:
        case i_TYPE_OPERATION_SETOP_DIFFERENCE:
        case i_TYPE_OPERATION_SETOP_INTERSECTION:
        {
            i_write_optree(optree->operands.setop_operation.node1, csmsave);
            i_write_optree(optree->operands.setop_operation.node2, csmsave);
            break;
        }

        case i_TYPE_OPERATION_SPLIT_ABOVE:
        case i_TYPE_OPERATION_SPLIT_BELOW:
        {
            csmsave_write_double(csmsave, optree->operands.split_plane.A);
            csmsave_write_double(csmsave, optree->operands.split_plane.B);
            csmsave_write_double(csmsave, optree->operands.split_plane.C);
            csmsave_write_double(csmsave, optree->operands.split_plane.D);
            
            i_write_optree(optree->operands.split_plane.node, csmsave);
            break;
        }
            
        case i_TYPE_OPERATION_GENERAL_TRANSFORM:
        {
            csmsave_write_double(csmsave, optree->operands.transform.Ux);
            csmsave_write_double(csmsave, optree->operands.transform.Uy);
            csmsave_write_double(csmsave, optree->operands.transform.Uz);
            csmsave_write_double(csmsave, optree->operands.transform.Dx);
                                 
            csmsave_write_double(csmsave, optree->operands.transform.Vx);
            csmsave_write_double(csmsave, optree->operands.transform.Vy);
            csmsave_write_double(csmsave, optree->operands.transform.Vz);
            csmsave_write_double(csmsave, optree->operands.transform.Dy);
            
            csmsave_write_double(csmsave, optree->operands.transform.Wx);
            csmsave_write_double(csmsave, optree->operands.transform.Wy);
            csmsave_write_double(csmsave, optree->operands.transform.Wz);
            csmsave_write_double(csmsave, optree->operands.transform.Dz);
            
            i_write_optree(optree->operands.transform.node, csmsave);
            break;
        }
            
        default_error();
    }
    
    csmsave_write_enum(csmsave, optree->evaluation_result);
    csmsave_write_optional_st(csmsave, optree->solid_result, csmsolid_write, csmsolid_t);
}

// ------------------------------------------------------------------------------------------

void csmoptree_write(const struct csmoptree_t *optree, struct csmsave_t *csmsave)
{
    assert_no_null(optree);
 
    csmsave_write_uchar(csmsave, i_FILE_VERSION);
    i_write_optree(optree, csmsave);
}

// ------------------------------------------------------------------------------------------

struct csmoptree_t *csmoptree_new_node_solid(struct csmsolid_t **solid)
{
    struct csmoptree_t *optree;
    
    optree = i_new_unevaluated_optree(i_TYPE_SOLID);
    assert_no_null(optree);
    
    optree->operands.solid = ASSIGN_POINTER_PP_NOT_NULL(solid, struct csmsolid_t);
    
    return optree;
}

// ------------------------------------------------------------------------------------------

struct csmoptree_t *csmoptree_new_node_boolean_union(struct csmoptree_t **node1, struct csmoptree_t **node2)
{
    struct csmoptree_t *optree;
    
    optree = i_new_unevaluated_optree(i_TYPE_OPERATION_SETOP_UNION);
    assert_no_null(optree);
    
    optree->operands.setop_operation.node1 = ASSIGN_POINTER_PP_NOT_NULL(node1, struct csmoptree_t);
    optree->operands.setop_operation.node2 = ASSIGN_POINTER_PP_NOT_NULL(node2, struct csmoptree_t);
    
    return optree;
}

// ------------------------------------------------------------------------------------------

struct csmoptree_t *csmoptree_new_node_boolean_intersection(struct csmoptree_t **node1, struct csmoptree_t **node2)
{
    struct csmoptree_t *optree;
    
    optree = i_new_unevaluated_optree(i_TYPE_OPERATION_SETOP_INTERSECTION);
    assert_no_null(optree);
    
    optree->operands.setop_operation.node1 = ASSIGN_POINTER_PP_NOT_NULL(node1, struct csmoptree_t);
    optree->operands.setop_operation.node2 = ASSIGN_POINTER_PP_NOT_NULL(node2, struct csmoptree_t);
    
    return optree;
}
                     
// ------------------------------------------------------------------------------------------

struct csmoptree_t *csmoptree_new_node_boolean_difference_1_minus_2(struct csmoptree_t **node1, struct csmoptree_t **node2)
{
    struct csmoptree_t *optree;
    
    optree = i_new_unevaluated_optree(i_TYPE_OPERATION_SETOP_DIFFERENCE);
    assert_no_null(optree);
    
    optree->operands.setop_operation.node1 = ASSIGN_POINTER_PP_NOT_NULL(node1, struct csmoptree_t);
    optree->operands.setop_operation.node2 = ASSIGN_POINTER_PP_NOT_NULL(node2, struct csmoptree_t);
    
    return optree;
}
                     
// ------------------------------------------------------------------------------------------

struct csmoptree_t *csmoptree_new_node_split_and_get_above(double A, double B, double C, double D, struct csmoptree_t **node)
{
    struct csmoptree_t *optree;
    
    optree = i_new_unevaluated_optree(i_TYPE_OPERATION_SPLIT_ABOVE);
    assert_no_null(optree);
    
    optree->operands.split_plane.A = A;
    optree->operands.split_plane.B = B;
    optree->operands.split_plane.C = C;
    optree->operands.split_plane.D = D;
    
    optree->operands.split_plane.node = ASSIGN_POINTER_PP_NOT_NULL(node, struct csmoptree_t);

    return optree;
}
                     
// ------------------------------------------------------------------------------------------

struct csmoptree_t *csmoptree_new_node_split_and_get_below(double A, double B, double C, double D, struct csmoptree_t **node)
{
    struct csmoptree_t *optree;
    
    optree = i_new_unevaluated_optree(i_TYPE_OPERATION_SPLIT_BELOW);
    assert_no_null(optree);
    
    optree->operands.split_plane.A = A;
    optree->operands.split_plane.B = B;
    optree->operands.split_plane.C = C;
    optree->operands.split_plane.D = D;
    
    optree->operands.split_plane.node = ASSIGN_POINTER_PP_NOT_NULL(node, struct csmoptree_t);
    
    return optree;
}

// ------------------------------------------------------------------------------------------

struct csmoptree_t *csmoptree_new_node_transform(
                        double Ux, double Uy, double Uz, double Dx,
                        double Vx, double Vy, double Vz, double Dy,
                        double Wx, double Wy, double Wz, double Dz,
                        struct csmoptree_t **node)
{
    struct csmoptree_t *optree;
    
    optree = i_new_unevaluated_optree(i_TYPE_OPERATION_GENERAL_TRANSFORM);
    assert_no_null(optree);
    
    optree->operands.transform.Ux = Ux;
    optree->operands.transform.Uy = Uy;
    optree->operands.transform.Uz = Uz;
    optree->operands.transform.Dx = Dx;
    
    optree->operands.transform.Vx = Vx;
    optree->operands.transform.Vy = Vy;
    optree->operands.transform.Vz = Vz;
    optree->operands.transform.Dy = Dy;
    
    optree->operands.transform.Wx = Wx;
    optree->operands.transform.Wy = Wy;
    optree->operands.transform.Wz = Wz;
    optree->operands.transform.Dz = Dz;
    
    optree->operands.transform.node = ASSIGN_POINTER_PP_NOT_NULL(node, struct csmoptree_t);
    
    return optree;
}

// ------------------------------------------------------------------------------------------

static void i_evaluate_node(struct csmoptree_t *node)
{
    assert_no_null(node);
    
    switch (node->type)
    {
        case i_TYPE_SOLID:
        {
            node->evaluation_result = CSMOPTREE_RESULT_OK;
            node->solid_result = csmsolid_duplicate(node->operands.solid);
            break;
        }
        
        case i_TYPE_OPERATION_SETOP_UNION:
        case i_TYPE_OPERATION_SETOP_DIFFERENCE:
        case i_TYPE_OPERATION_SETOP_INTERSECTION:
        {
            i_evaluate_node(node->operands.setop_operation.node1);
            i_evaluate_node(node->operands.setop_operation.node2);
            
            if (node->operands.setop_operation.node1->evaluation_result != CSMOPTREE_RESULT_OK)
            {
                node->evaluation_result = node->operands.setop_operation.node1->evaluation_result;
                node->solid_result = NULL;
            }
            else if (node->operands.setop_operation.node2->evaluation_result != CSMOPTREE_RESULT_OK)
            {
                node->evaluation_result = node->operands.setop_operation.node2->evaluation_result;
                node->solid_result = NULL;
            }
            else
            {
                enum csmsetop_opresult_t setop_result;
                
                if (node->type == i_TYPE_OPERATION_SETOP_UNION)
                {
                    setop_result = csmsetop_union_A_and_B(
                                        node->operands.setop_operation.node1->solid_result, node->operands.setop_operation.node2->solid_result,
                                        &node->solid_result);
                }
                else if (node->type == i_TYPE_OPERATION_SETOP_INTERSECTION)
                {
                    setop_result = csmsetop_intersection_A_and_B(
                                        node->operands.setop_operation.node1->solid_result, node->operands.setop_operation.node2->solid_result,
                                        &node->solid_result);                    
                }
                else
                {
                    assert(node->type == i_TYPE_OPERATION_SETOP_DIFFERENCE);
                    
                    setop_result = csmsetop_difference_A_minus_B(
                                        node->operands.setop_operation.node1->solid_result, node->operands.setop_operation.node2->solid_result,
                                        &node->solid_result);
                }
                
                switch (setop_result)
                {
                    case CSMSETOP_OPRESULT_OK:
                        
                        node->evaluation_result = CSMOPTREE_RESULT_OK;
                        break;
                        
                    case CSMSETOP_OPRESULT_NON_MANIFOLD_OPERAND:
                        
                        node->evaluation_result = CSMOPTREE_RESULT_SETOP_NON_MANIFOLD;
                        break;
                        
                    case CSMSETOP_OPRESULT_IMPROPER_INTERSECTIONS:
                        
                        node->evaluation_result = CSMOPTREE_RESULT_SETOP_IMPROPER_INTERSECTIONS;
                        break;
                        
                    default_error();
                }
            }
            break;
        }

        case i_TYPE_OPERATION_SPLIT_ABOVE:
        case i_TYPE_OPERATION_SPLIT_BELOW:
        {
            i_evaluate_node(node->operands.split_plane.node);
            
            if (node->operands.split_plane.node->evaluation_result != CSMOPTREE_RESULT_OK)
            {
                node->evaluation_result = node->operands.split_plane.node->evaluation_result;
                node->solid_result = NULL;
            }
            else
            {
                enum csmsplit_opresult_t split_result;
                struct csmsolid_t *solid_above, *solid_below;
                
                split_result = csmsplit_split_solid(
                                        node->operands.split_plane.node->solid_result,
                                        node->operands.split_plane.A, node->operands.split_plane.B, node->operands.split_plane.C, node->operands.split_plane.D,
                                        &solid_above, &solid_below);
                
                switch (split_result)
                {
                    case CSMSPLIT_OPRESULT_OK:
                    {
                        if (node->type == i_TYPE_OPERATION_SPLIT_ABOVE)
                        {
                            if (solid_above == NULL)
                            {
                                node->evaluation_result = CSMOPTREE_RESULT_SPLIT_NO_SPLIT;
                                node->solid_result = NULL;
                            }
                            else
                            {
                                node->evaluation_result = CSMOPTREE_RESULT_OK;
                                node->solid_result = ASSIGN_POINTER_PP_NOT_NULL(&solid_above, struct csmsolid_t);
                            }
                            
                            csmsolid_free(&solid_below);
                        }
                        else
                        {
                            assert(node->type == i_TYPE_OPERATION_SPLIT_BELOW);
                            
                            if (solid_below == NULL)
                            {
                                node->evaluation_result = CSMOPTREE_RESULT_SPLIT_NO_SPLIT;
                                node->solid_result = NULL;
                            }
                            else
                            {
                                node->evaluation_result = CSMOPTREE_RESULT_OK;
                                node->solid_result = ASSIGN_POINTER_PP_NOT_NULL(&solid_below, struct csmsolid_t);
                            }
                            
                            csmsolid_free(&solid_above);
                        }
                        break;
                    }
                        
                    case CSMSPLIT_OPRESULT_NO:
                        
                        node->evaluation_result = CSMOPTREE_RESULT_SPLIT_NO_SPLIT;
                        node->solid_result = NULL;
                        break;
                        
                    case CSMSPLIT_OPRESULT_IMPROPER_CUT:
                        
                        node->evaluation_result = CSMOPTREE_RESULT_SPLIT_IMPROPER_CUT;
                        node->solid_result = NULL;
                        break;
                        
                    default_error();
                }
                
            }
            break;
        }
            
        case i_TYPE_OPERATION_GENERAL_TRANSFORM:
        {
            i_evaluate_node(node->operands.transform.node);
            
            if (node->operands.transform.node->evaluation_result != CSMOPTREE_RESULT_OK)
            {
                node->evaluation_result = node->operands.transform.node->evaluation_result;
                node->solid_result = NULL;
            }
            else
            {
                node->evaluation_result = CSMOPTREE_RESULT_OK;
                node->solid_result = csmsolid_duplicate(node->operands.transform.node->solid_result);
                
                csmsolid_general_transform(
                        node->solid_result,
                        node->operands.transform.Ux, node->operands.transform.Uy, node->operands.transform.Uz, node->operands.transform.Dx,
                        node->operands.transform.Vx, node->operands.transform.Vy, node->operands.transform.Vz, node->operands.transform.Dy,
                        node->operands.transform.Wx, node->operands.transform.Wy, node->operands.transform.Wz, node->operands.transform.Dz);
            }
            break;
        }
            
        default_error();
    }
}

// ------------------------------------------------------------------------------------------

static void i_assign_node_id(struct csmoptree_t *node, unsigned long *id_new_element)
{
    assert_no_null(node);
    
    switch (node->type)
    {
        case i_TYPE_SOLID:
            break;
        
        case i_TYPE_OPERATION_SETOP_UNION:
        case i_TYPE_OPERATION_SETOP_DIFFERENCE:
        case i_TYPE_OPERATION_SETOP_INTERSECTION:
        
            i_assign_node_id(node->operands.setop_operation.node1, id_new_element);
            i_assign_node_id(node->operands.setop_operation.node2, id_new_element);
            break;

        case i_TYPE_OPERATION_SPLIT_ABOVE:
        case i_TYPE_OPERATION_SPLIT_BELOW:
        
            i_assign_node_id(node->operands.split_plane.node, id_new_element);
            break;
            
        case i_TYPE_OPERATION_GENERAL_TRANSFORM:
        
            i_assign_node_id(node->operands.transform.node, id_new_element);
            break;
            
        default_error();
    }
    
    node->node_id = csmid_new_id(id_new_element, NULL);
}

// ------------------------------------------------------------------------------------------

enum csmoptree_result_t csmoptree_evaluate(struct csmoptree_t *node, struct csmsolid_t **solid)
{
    struct csmsolid_t *solid_loc;
    
    assert_no_null(node);
    assert_no_null(solid);
    
    if (node->evaluation_result == i_UNEVALUATED_RESULT)
    {
        unsigned long id_new_element;
        
        id_new_element = 0;
        i_assign_node_id(node, &id_new_element);
        
        i_evaluate_node(node);
    }
        
    if (node->solid_result != NULL)
        solid_loc = csmsolid_duplicate(node->solid_result);
    else
        solid_loc = NULL;
    
    *solid = solid_loc;
 
    return node->evaluation_result;
}

// ------------------------------------------------------------------------------------------

void csmoptree_clean_results(struct csmoptree_t *node)
{
    assert_no_null(node);
    
    if (node->evaluation_result != i_UNEVALUATED_RESULT)
    {
        switch (node->type)
        {
            case i_TYPE_SOLID:
                break;
            
            case i_TYPE_OPERATION_SETOP_UNION:
            case i_TYPE_OPERATION_SETOP_DIFFERENCE:
            case i_TYPE_OPERATION_SETOP_INTERSECTION:
            
                csmoptree_clean_results(node->operands.setop_operation.node1);
                csmoptree_clean_results(node->operands.setop_operation.node2);
                break;

            case i_TYPE_OPERATION_SPLIT_ABOVE:
            case i_TYPE_OPERATION_SPLIT_BELOW:
            
                csmoptree_clean_results(node->operands.split_plane.node);
                break;
                
            case i_TYPE_OPERATION_GENERAL_TRANSFORM:
            
                csmoptree_clean_results(node->operands.transform.node);
                break;
                
            default_error();
        }
    
        node->evaluation_result = i_UNEVALUATED_RESULT;
        csmsolid_free(&node->solid_result);
    }
}
