//
//  csmeuler.h
//  cysolidmodeling
//
//  Created by Manuel Fernández on 24/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

struct csmsolid_t *csmeuler_mvfs(double x, double y, double z, unsigned long *id_nuevo_elemento, struct csmhedge_t **hedge_opc);
