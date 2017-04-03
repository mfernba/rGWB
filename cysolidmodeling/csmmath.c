//
//  csmmath.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmmath.inl"

#include <math.h>

// ------------------------------------------------------------------------------------------

CYBOOL csmmath_equal_coords(
                        double x1, double y1, double z1,
                        double x2, double y2, double z2,
                        double epsilon)
{
    if (fabs(x1 - x2) > epsilon)
        return FALSO;
    
    if (fabs(y1 - y2) > epsilon)
        return FALSO;
    
    if (fabs(z1 - z2) > epsilon)
        return FALSO;
    
    return CIERTO;
}
