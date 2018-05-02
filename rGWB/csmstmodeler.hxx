//
//  csmstmodeler.hxx
//  rGWB
//
//  Created by Manuel Fernández on 16/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#ifndef csmstmodeler_hxx
#define csmstmodeler_hxx

enum csmstmodeler_result_t
{
    CSMSTMODELER_RESULT_OK = 0,
    CSMSTMODELER_RESULT_IMPROPER_INTERSECTIONS = 1,
    CSMSTMODELER_RESULT_NON_MANIFOLD_OPERAND = 2,
    CSMSTMODELER_RESULT_INCORRECT_NUMBER_OF_OPERANDS = 3,
    CSMSTMODELER_RESULT_INVALID_ARGUMENT = 4,
    CSMSTMODELER_RESULT_NOTHING_DONE = 5
};

#endif /* csmstmodeler_h */
