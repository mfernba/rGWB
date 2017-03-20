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

#define CYBOOL char
#define CIERTO 1
#define FALSO 0
#define CONSTRUCTOR(type, name, params) type name params

#define ES_CIERTO(condicion) (condicion) ? CIERTO: FALSO

#define ASIGNA_OPC(a, b) if ((a) != NULL) *(a) = (b)

struct csmnode_t;
struct csmvertex_t;
struct csmhedge_t;
struct csmedge_t;
struct csmloop_t;
struct csmface_t;
struct csmsolid_t;
struct csmsolidtopology_t;

enum csmedge_lado_hedge_t;

struct csmbbox_t;

#include "csmhashtb.hxx"
struct csmhashtb(csmface_t);
struct csmhashtb(csmedge_t);
struct csmhashtb(csmvertex_t);
struct csmhashtb(csmhedge_t);

#endif /* csmfwddecl_hxx */
