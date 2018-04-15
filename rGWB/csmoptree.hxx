//
//  csmoptree.hxx
//  rGWB
//
//  Created by Manuel Fernández on 15/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#ifndef csmoptree_hxx
#define csmoptree_hxx

enum csmoptree_result_t
{
    CSMOPTREE_RESULT_OK = 1,
    CSMOPTREE_RESULT_SPLIT_NO_SPLIT = 2,
    CSMOPTREE_RESULT_SPLIT_IMPROPER_CUT = 3,
    CSMOPTREE_RESULT_SETOP_NON_MANIFOLD = 4,
    CSMOPTREE_RESULT_SETOP_IMPROPER_INTERSECTIONS = 5
};

#endif /* csmoptree_hxx */
