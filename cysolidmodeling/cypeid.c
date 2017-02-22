//
//  cypeid.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/2/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "cypeid.h"

#include "cyassert.h"
#include <stdlib.h>

// ----------------------------------------------------------------------------------------------------

unsigned long cypeid_nuevo_id(unsigned long *id_nuevo_elemento, unsigned long *id_asignado_opc)
{
    unsigned long id;
    
    assert_no_null(id_nuevo_elemento);
    
    id = (*id_nuevo_elemento)++;
    
    if (id_asignado_opc != NULL)
        *id_asignado_opc = id;
    
    return id;
}
