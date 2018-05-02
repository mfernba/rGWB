//
//  csmid.c
//  rGWB
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmid.inl"

#include "csmfwddecl.hxx"

#ifdef __STANDALONE_DISTRIBUTABLE
#include "csmassert.inl"
#else
#include "cyassert.h"
#endif

// ----------------------------------------------------------------------------------------------------

unsigned long csmid_new_id(unsigned long *id_new_element, unsigned long *assigned_id_opt)
{
    unsigned long new_id;
    
    assert_no_null(id_new_element);
    
    new_id = (*id_new_element)++;
    ASSIGN_OPTIONAL_VALUE(assigned_id_opt, new_id);
    
    return new_id;
}
