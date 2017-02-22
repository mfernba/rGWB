//
//  csmutilptr.h
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#define csmutilptr_set_ptr_next_or_prev(ptr, next_or_prev_vertex)\
(\
{\
    assert_no_null((ptr));\
    \
    if (*(ptr) != NULL)\
        csmnode_unretain(CSMNODE((*(ptr))));\
    \
    *(ptr) = (next_or_prev_vertex);\
    \
    if ((next_or_prev_vertex) != NULL)\
        csmnode_retain(CSMNODE((next_or_prev_vertex)));\
})
