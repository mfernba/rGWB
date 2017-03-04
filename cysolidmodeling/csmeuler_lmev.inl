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
                unsigned long *id_nuevo_elemento,
                struct csmedge_t **edge_opc, struct csmvertex_t **vertex_opc);
