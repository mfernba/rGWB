//
//  csmeuler_lmev.h
//  cysolidmodeling
//
//  Created by Manuel Fernández on 28/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

void csmeuler_lmev(
                struct csmhedge_t *he1, struct csmhedge_t *he2,
                double x, double y, double z,
                struct csmvertex_t **vertex_opc,
                struct csmedge_t **edge_opc,
                struct csmhedge_t **hedge_from_new_vertex_opc,
                struct csmhedge_t **hedge_to_new_vertex_opc);

void csmeuler_lmev_strut_edge(
                struct csmhedge_t *he,
                double x, double y, double z,
                struct csmhedge_t **hedge_from_new_vertex_opc);
