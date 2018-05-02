//
//  csmstmodeler.c
//  rGWB
//
//  Created by Manuel Fernández on 16/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmstmodeler.h"

#include "csmarrayc.inl"
#include "csmid.inl"
#include "csmsetop.h"
#include "csmsetop.hxx"
#include "csmsolid.h"
#include "csmsplit.h"
#include "csmsplit.hxx"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmmem.inl"
#include "csmassert.inl"
#else
#include "cyassert.h"
#include "cypespy.h"
#endif

struct i_operand_t
{
    unsigned long solid_id;
    struct csmsolid_t *solid;
};

struct csmstmodeler_t
{
    unsigned long id_new_element;
    csmArrayStruct(i_operand_t) *operands;
};

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_operand_t *, i_new_operand, (unsigned long solid_id, struct csmsolid_t **solid))
{
    struct i_operand_t *operand;
    
    operand = MALLOC(struct i_operand_t);
    
    operand->solid_id = solid_id;
    operand->solid = ASIGNA_PUNTERO_PP_NO_NULL(solid, struct csmsolid_t);
    
    return operand;
}

// ------------------------------------------------------------------------------------------

static void i_free_operand(struct i_operand_t **operand)
{
    assert_no_null(operand);
    assert_no_null(*operand);
    
    csmsolid_free(&(*operand)->solid);
    
    FREE_PP(operand, struct i_operand_t);
}

// ------------------------------------------------------------------------------------------

static CSMBOOL i_is_operand_with_id(const struct i_operand_t *operand, const unsigned long *solid_id)
{
    assert_no_null(operand);
    assert_no_null(solid_id);
    
    return IS_TRUE(operand->solid_id == *solid_id);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmstmodeler_t *, i_new, (
                        unsigned long id_new_element,
                        csmArrayStruct(i_operand_t) **operands))
{
    struct csmstmodeler_t *modeler;
    
    modeler = MALLOC(struct csmstmodeler_t);
    
    modeler->id_new_element = id_new_element;
    modeler->operands = ASIGNA_PUNTERO_PP_NO_NULL(operands, csmArrayStruct(i_operand_t));
    
    return modeler;
}

// ------------------------------------------------------------------------------------------

struct csmstmodeler_t *csmstmodeler_new(void)
{
    unsigned long id_new_element;
    csmArrayStruct(i_operand_t) *operands;
    
    id_new_element = 0;
    operands = csmarrayc_new_st_array(0, i_operand_t);
    
    return i_new(id_new_element, &operands);
}

// ------------------------------------------------------------------------------------------

void csmstmodeler_free(struct csmstmodeler_t **modeler)
{
    assert_no_null(modeler);
    assert_no_null(*modeler);
    
    csmarrayc_free_st(&(*modeler)->operands, i_operand_t, i_free_operand);
    
    FREE_PP(modeler, struct csmstmodeler_t);
}

// ------------------------------------------------------------------------------------------

void csmstmodeler_clear_stack(struct csmstmodeler_t *modeler)
{
    assert_no_null(modeler);
    
    csmarrayc_free_st(&modeler->operands, i_operand_t, i_free_operand);
    modeler->operands = csmarrayc_new_st_array(0, i_operand_t);
}

// ------------------------------------------------------------------------------------------

unsigned long csmstmodeler_push_solid(struct csmstmodeler_t *modeler, struct csmsolid_t **solid)
{
    unsigned long solid_id;
    struct i_operand_t *operand;
    
    assert_no_null(modeler);
    
    solid_id = csmid_new_id(&modeler->id_new_element, NULL);
    
    operand = i_new_operand(solid_id, solid);
    csmarrayc_append_element_st(modeler->operands, operand, i_operand_t);
    
    return solid_id;
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_push_solid_by_id(struct csmstmodeler_t *modeler, unsigned long solid_id)
{
    enum csmstmodeler_result_t result;
    unsigned long idx;
    
    assert_no_null(modeler);
    
    if (csmarrayc_contains_element_st(modeler->operands, i_operand_t, &solid_id, unsigned long, i_is_operand_with_id, &idx) == CSMFALSE)
    {
        result = CSMSTMODELER_RESULT_INVALID_ARGUMENT;
    }
    else
    {
        struct i_operand_t *operand;
        
        result = CSMSTMODELER_RESULT_OK;
        
        operand = csmarrayc_get_st(modeler->operands, idx, i_operand_t);
        csmarrayc_delete_element_st(modeler->operands, idx, i_operand_t, NULL);
        
        csmarrayc_append_element_st(modeler->operands, operand, i_operand_t);
    }
    
    return result;
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_duplicate_solid(struct csmstmodeler_t *modeler, unsigned long solid_id, unsigned long *solid_id_opt)
{
    enum csmstmodeler_result_t result;
    unsigned long idx;
    
    assert_no_null(modeler);
    
    if (csmarrayc_contains_element_st(modeler->operands, i_operand_t, &solid_id, unsigned long, i_is_operand_with_id, &idx) == CSMFALSE)
    {
        result = CSMSTMODELER_RESULT_INVALID_ARGUMENT;
        ASSIGN_OPTIONAL_VALUE(solid_id_opt, ULONG_MAX);
    }
    else
    {
        struct i_operand_t *operand;
        unsigned long new_solid_id;
        struct csmsolid_t *solid_copy;
        struct i_operand_t *new_operand;
        
        result = CSMSTMODELER_RESULT_OK;
        
        operand = csmarrayc_get_st(modeler->operands, idx, i_operand_t);
        assert_no_null(operand);
        
        new_solid_id = csmid_new_id(&modeler->id_new_element, solid_id_opt);
        solid_copy = csmsolid_duplicate(operand->solid);
    
        new_operand = i_new_operand(new_solid_id, &solid_copy);
        csmarrayc_append_element_st(modeler->operands, new_operand, i_operand_t);
    }
    
    return result;
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_get_solid_reference(const struct csmstmodeler_t *modeler, unsigned long solid_id, struct csmsolid_t **solid_reference)
{
    enum csmstmodeler_result_t result;
    unsigned long idx;
    
    assert_no_null(modeler);
    assert_no_null(solid_reference);
    
    if (csmarrayc_contains_element_st(modeler->operands, i_operand_t, &solid_id, unsigned long, i_is_operand_with_id, &idx) == CSMFALSE)
    {
        result = CSMSTMODELER_RESULT_INVALID_ARGUMENT;
        *solid_reference = NULL;
    }
    else
    {
        struct i_operand_t *operand;
        
        result = CSMSTMODELER_RESULT_OK;
        
        operand = csmarrayc_get_st(modeler->operands, idx, i_operand_t);
        assert_no_null(operand);
        
        *solid_reference = operand->solid;
    }
    
    return result;
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_get_solid_copy(const struct csmstmodeler_t *modeler, unsigned long solid_id, struct csmsolid_t **solid_copy)
{
    enum csmstmodeler_result_t result;
    unsigned long idx;
    
    assert_no_null(modeler);
    assert_no_null(solid_copy);
    
    if (csmarrayc_contains_element_st(modeler->operands, i_operand_t, &solid_id, unsigned long, i_is_operand_with_id, &idx) == CSMFALSE)
    {
        result = CSMSTMODELER_RESULT_INVALID_ARGUMENT;
        *solid_copy = NULL;
    }
    else
    {
        struct i_operand_t *operand;
        
        result = CSMSTMODELER_RESULT_OK;
        
        operand = csmarrayc_get_st(modeler->operands, idx, i_operand_t);
        assert_no_null(operand);
        
        *solid_copy = csmsolid_duplicate(operand->solid);
    }
    
    return result;
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_apply_move(struct csmstmodeler_t *modeler, double dx, double dy, double dz)
{
    enum csmstmodeler_result_t result;
    unsigned long no_operands;

    assert_no_null(modeler);
    
    no_operands = csmarrayc_count_st(modeler->operands, i_operand_t);
    
    if (no_operands == 0)
    {
        result = CSMSTMODELER_RESULT_INCORRECT_NUMBER_OF_OPERANDS;
    }
    else
    {
        struct i_operand_t *operand;
        
        result = CSMSTMODELER_RESULT_OK;
        
        operand = csmarrayc_get_st(modeler->operands, no_operands - 1, i_operand_t);
        assert_no_null(operand);
        
        csmsolid_move(operand->solid, dx, dy, dz);
    }
    
    return result;
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_apply_scale(struct csmstmodeler_t *modeler, double sx, double sy, double sz)
{
    enum csmstmodeler_result_t result;
    unsigned long no_operands;

    assert_no_null(modeler);
    
    no_operands = csmarrayc_count_st(modeler->operands, i_operand_t);
    
    if (no_operands == 0)
    {
        result = CSMSTMODELER_RESULT_INCORRECT_NUMBER_OF_OPERANDS;
    }
    else
    {
        struct i_operand_t *operand;
        
        result = CSMSTMODELER_RESULT_OK;
        
        operand = csmarrayc_get_st(modeler->operands, no_operands - 1, i_operand_t);
        assert_no_null(operand);
        
        csmsolid_scale(operand->solid, sx, sy, sz);
    }
    
    return result;
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_apply_rotation(
                        struct csmstmodeler_t *modeler,
                        double angle_radians,
                        double Xo, double Yo, double Zo, double Ux, double Uy, double Uz)
{
    enum csmstmodeler_result_t result;
    unsigned long no_operands;

    assert_no_null(modeler);
    
    no_operands = csmarrayc_count_st(modeler->operands, i_operand_t);
    
    if (no_operands == 0)
    {
        result = CSMSTMODELER_RESULT_INCORRECT_NUMBER_OF_OPERANDS;
    }
    else
    {
        struct i_operand_t *operand;
        
        result = CSMSTMODELER_RESULT_OK;
        
        operand = csmarrayc_get_st(modeler->operands, no_operands - 1, i_operand_t);
        assert_no_null(operand);
        
        csmsolid_rotate(operand->solid, angle_radians, Xo, Yo, Zo, Ux, Uy, Uz);
    }
    
    return result;
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_apply_general_transform(
                        struct csmstmodeler_t *modeler,
                        double Ux, double Uy, double Uz, double Dx,
                        double Vx, double Vy, double Vz, double Dy,
                        double Wx, double Wy, double Wz, double Dz)
{
    enum csmstmodeler_result_t result;
    unsigned long no_operands;

    assert_no_null(modeler);
    
    no_operands = csmarrayc_count_st(modeler->operands, i_operand_t);
    
    if (no_operands == 0)
    {
        result = CSMSTMODELER_RESULT_INCORRECT_NUMBER_OF_OPERANDS;
    }
    else
    {
        struct i_operand_t *operand;
        
        result = CSMSTMODELER_RESULT_OK;
        
        operand = csmarrayc_get_st(modeler->operands, no_operands - 1, i_operand_t);
        assert_no_null(operand);
        
        csmsolid_general_transform(
                        operand->solid,
                        Ux, Uy, Uz, Dx,
                        Vx, Vy, Vz, Dy,
                        Wx, Wy, Wz, Dz);
    }
    
    return result;
}

// ------------------------------------------------------------------------------------------

static enum csmstmodeler_result_t i_do_split_and_push(
                        unsigned long *id_new_element,
                        csmArrayStruct(i_operand_t) *operands,
                        double A, double B, double C, double D,
                        CSMBOOL push_above,
                        unsigned long *solid_id_opt)
{
    enum csmstmodeler_result_t result;
    unsigned long no_operands;

    no_operands = csmarrayc_count_st(operands, i_operand_t);
    
    if (no_operands == 0)
    {
        result = CSMSTMODELER_RESULT_INCORRECT_NUMBER_OF_OPERANDS;
    }
    else
    {
        struct i_operand_t *operand;
        struct csmsolid_t *solid_above, *solid_below;
        enum csmsplit_opresult_t split_result;
        
        operand = csmarrayc_get_st(operands, no_operands - 1, i_operand_t);
        assert_no_null(operand);
        
        split_result = csmsplit_split_solid(operand->solid, A, B, C, D, &solid_above, &solid_below);
        
        switch (split_result)
        {
            case CSMSPLIT_OPRESULT_OK:
            {
                unsigned long solid_id;
                struct i_operand_t *new_operand;
                
                result = CSMSTMODELER_RESULT_OK;
                
                solid_id = csmid_new_id(id_new_element, solid_id_opt);
                
                if (push_above == CSMTRUE)
                {
                    new_operand = i_new_operand(solid_id, &solid_above);
                    csmsolid_free(&solid_below);
                }
                else
                {
                    new_operand = i_new_operand(solid_id, &solid_below);
                    csmsolid_free(&solid_above);
                }
                
                csmarrayc_append_element_st(operands, new_operand, i_operand_t);
                break;
            }
                
            case CSMSPLIT_OPRESULT_NO:
                
                result = CSMSTMODELER_RESULT_NOTHING_DONE;
                ASSIGN_OPTIONAL_VALUE(solid_id_opt, ULONG_MAX);
                break;
                
            case CSMSPLIT_OPRESULT_IMPROPER_CUT:
                
                result = CSMSTMODELER_RESULT_IMPROPER_INTERSECTIONS;
                ASSIGN_OPTIONAL_VALUE(solid_id_opt, ULONG_MAX);
                break;
                
            default_error();
        }
    }
    
    return result;
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_do_split_and_push_above(struct csmstmodeler_t *modeler, double A, double B, double C, double D, unsigned long *solid_id_opt)
{
    CSMBOOL push_above;
    
    assert_no_null(modeler);
    
    push_above = CSMTRUE;
    return i_do_split_and_push(&modeler->id_new_element, modeler->operands, A, B, C, D, push_above, solid_id_opt);
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_do_split_and_push_below(struct csmstmodeler_t *modeler, double A, double B, double C, double D, unsigned long *solid_id_opt)
{
    CSMBOOL push_above;
    
    assert_no_null(modeler);
    
    push_above = CSMFALSE;
    return i_do_split_and_push(&modeler->id_new_element, modeler->operands, A, B, C, D, push_above, solid_id_opt);
}

// ------------------------------------------------------------------------------------------

static enum csmstmodeler_result_t i_do_boolean_operation(
                        unsigned long *id_new_element,
                        csmArrayStruct(i_operand_t) *operands,
                        enum csmsetop_opresult_t (*func_boolean_operation)(const struct csmsolid_t *solid_a, const struct csmsolid_t *solid_b, struct csmsolid_t **solid_res),
                        unsigned long *solid_id_opt)
{
    enum csmstmodeler_result_t result;
    unsigned long no_operands;
    
    assert_no_null(func_boolean_operation);

    no_operands = csmarrayc_count_st(operands, i_operand_t);
    
    if (no_operands < 2)
    {
        result = CSMSTMODELER_RESULT_INCORRECT_NUMBER_OF_OPERANDS;
    }
    else
    {
        struct i_operand_t *operand1, *operand2;
        enum csmsetop_opresult_t setop_result;
        struct csmsolid_t *solid_res;
        
        operand1 = csmarrayc_get_st(operands, no_operands - 1, i_operand_t);
        assert_no_null(operand1);
        
        operand2 = csmarrayc_get_st(operands, no_operands - 2, i_operand_t);
        assert_no_null(operand2);
        
        setop_result = func_boolean_operation(operand1->solid, operand2->solid, &solid_res);
        
        switch (setop_result)
        {
            case CSMSETOP_OPRESULT_OK:
            {
                unsigned long solid_id;
                struct i_operand_t *new_operand;
                
                result = CSMSTMODELER_RESULT_OK;
                
                solid_id = csmid_new_id(id_new_element, solid_id_opt);
                
                new_operand = i_new_operand(solid_id, &solid_res);
                csmarrayc_append_element_st(operands, new_operand, i_operand_t);
                break;
            }
                
            case CSMSETOP_OPRESULT_NON_MANIFOLD_OPERAND:

                result = CSMSTMODELER_RESULT_NON_MANIFOLD_OPERAND;
                ASSIGN_OPTIONAL_VALUE(solid_id_opt, ULONG_MAX);
                break;
                
            case CSMSETOP_OPRESULT_IMPROPER_INTERSECTIONS:
                
                result = CSMSTMODELER_RESULT_IMPROPER_INTERSECTIONS;
                ASSIGN_OPTIONAL_VALUE(solid_id_opt, ULONG_MAX);
                break;
                
            default_error();
        }
    }
    
    return result;
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_do_union(struct csmstmodeler_t *modeler, unsigned long *solid_id_opt)
{
    assert_no_null(modeler);
    return i_do_boolean_operation(&modeler->id_new_element, modeler->operands, csmsetop_union_A_and_B, solid_id_opt);
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_do_intersection(struct csmstmodeler_t *modeler, unsigned long *solid_id_opt)
{
    assert_no_null(modeler);
    return i_do_boolean_operation(&modeler->id_new_element, modeler->operands, csmsetop_intersection_A_and_B, solid_id_opt);
}

// ------------------------------------------------------------------------------------------

enum csmstmodeler_result_t csmstmodeler_do_difference(struct csmstmodeler_t *modeler, unsigned long *solid_id_opt)
{
    assert_no_null(modeler);
    return i_do_boolean_operation(&modeler->id_new_element, modeler->operands, csmsetop_difference_A_minus_B, solid_id_opt);
}







