//
//  csmsetop.hxx
//  rGWB
//
//  Created by Manuel Fernández on 29/12/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#ifndef csmsetop_hxx
#define csmsetop_hxx

enum csmsetop_opresult_t
{
    CSMSETOP_OPRESULT_OK,
    CSMSETOP_OPRESULT_NON_MANIFOLD_OPERAND,
    CSMSETOP_OPRESULT_IMPROPER_INTERSECTIONS
};

typedef enum csmsetop_opresult_t csmsetop_opresult;

#endif /* csmsetop_hxx */
