//
//  csmloop_debug.h
//  cysolidmodeling
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

void csmloop_debug_print_info_debug(
                        struct csmloop_t *loop,
                        CSMBOOL is_outer_loop,
                        CSMBOOL with_loop_area, double loop_area,
                        CSMBOOL assert_si_no_es_integro);

