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
    {\
        assert((*(ptr))->num_referencias >= 2);\
        (*(ptr))->num_referencias--;\
    }\
    \
    *(ptr) = (next_or_prev_vertex);\
    \
    if ((next_or_prev_vertex) != NULL)\
        (next_or_prev_vertex)->num_referencias++;\
})
