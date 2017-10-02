//
//  csmfwddecl.h
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#ifndef csmfwddecl_hxx
#define csmfwddecl_hxx

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#ifndef __cplusplus
#define CSMBOOL char
#define CSMTRUE 1
#define CSMFALSE 0
#endif

#ifdef __cplusplus
#define CSMBOOL bool
#define CSMTRUE true
#define CSMFALSE false
#endif

#define CONSTRUCTOR(type, name, params) type name params

#define IS_TRUE(condicion) (condicion) ? CSMTRUE: CSMFALSE

#define ASSIGN_OPTIONAL_VALUE(a, b) if ((a) != NULL) *(a) = (b)

#define DLL_CYSOLIDMODELING

#define comparac_t ecomparacion_t

#define INVERT_BOOLEAN(valor) (((valor) == CSMTRUE) ? CSMFALSE: CSMTRUE)

#ifndef __cplusplus
#define comparac_IGUALES ECOMPARACION_IGUALES
#define comparac_PRIMERO_MENOR ECOMPARACION_PRIMERO_MENOR
#define comparac_PRIMERO_MAYOR ECOMPARACION_PRIMERO_MAYOR
#endif

struct csmnode_t;
struct csmvertex_t;
struct csmhedge_t;
struct csmedge_t;
struct csmloop_t;
struct csmface_t;
struct csmsolid_t;
struct csmtransform_t;
    
#ifdef __cplusplus
enum csmedge_lado_hedge_t: int;
enum csmsetop_operation_t: int;
enum csmsetop_classify_resp_solid_t: int;
enum csmsetop_a_vs_b_t: int;
#else
enum csmedge_lado_hedge_t;
enum csmsetop_operation_t;
enum csmsetop_classify_resp_solid_t;
enum csmsetop_a_vs_b_t;
#endif

struct csmbbox_t;
struct csmtransform_t;
    
struct bsgraphics2_t;
struct csmviewer_t;

#include "csmhashtb.hxx"
struct csmhashtb(csmface_t);
struct csmhashtb(csmedge_t);
struct csmhashtb(csmvertex_t);
struct csmhashtb(csmhedge_t);


struct gccontorno_t;
struct bsmaterial_t;
    
#include "csmmath.tli"

#ifdef __cplusplus
}
#endif

#endif /* csmfwddecl_hxx */
