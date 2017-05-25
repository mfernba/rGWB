//
//  csmfwddecl.h
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#ifndef csmfwddecl_hxx
#define csmfwddecl_hxx

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#ifndef __cplusplus
#define CYBOOL char
#define CIERTO 1
#define FALSO 0
#endif

#ifdef __cplusplus
#define CYBOOL bool
#define CIERTO true
#define FALSO false
#endif

#define CONSTRUCTOR(type, name, params) type name params

#define ES_CIERTO(condicion) (condicion) ? CIERTO: FALSO

#define ASIGNA_OPC(a, b) if ((a) != NULL) *(a) = (b)

#define DLL_CYSOLIDMODELING

#define comparac_t ecomparacion_t

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
enum csmedge_lado_hedge_t;
enum csmsetop_operation_t;
enum csmsetop_classify_resp_solid_t;
enum csmsetop_a_vs_b_t;

struct csmbbox_t;
struct csmtransform_t;


#include "csmhashtb.hxx"
struct csmhashtb(csmface_t);
struct csmhashtb(csmedge_t);
struct csmhashtb(csmvertex_t);
struct csmhashtb(csmhedge_t);


struct gccontorno_t;

#include "csmmath.tli"

#endif /* csmfwddecl_hxx */
