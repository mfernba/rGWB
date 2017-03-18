//
//  csmhashtb.h
//  cysolidmodeling
//
//  Created by Manuel Fernández on 16/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#ifndef csmhashtb_h
#define csmhashtb_h

#define csmhashtb(tipo) csmhashtb__##tipo
#define csmhashtb_iterator(tipo) csmhashtb__iterator_##tipo

struct csmhashtb_item_ptr_t;

typedef void (*csmhashtb_FPtr_free_item_ptr)(struct csmhashtb_item_ptr_t **ptr);
#define CSMHASHTB_CHECK_FUNC_FREE_ITEM_PTR(func, ptr_type) ((void(*)(struct ptr_type **))func == func)

#endif /* csmhashtb_h */
