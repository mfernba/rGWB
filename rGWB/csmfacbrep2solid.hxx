//
//  csmfacbrep2solid.hxx
//  rGWB
//
//  Created by Manuel Fernandez on 21/4/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#ifndef __CSMFACBREP2SOLID_HXX_
#define __CSMFACBREP2SOLID_HXX_

enum csmfacbrep2solid_result_t
{
    CSMFACBREP2SOLID_RESULT_OK = 1,
    CSMFACBREP2SOLID_RESULT_EMPTY_SOLID = 2,
    CSMFACBREP2SOLID_RESULT_NON_MANIFOLD_FACETED_BREP = 3,
    CSMFACBREP2SOLID_RESULT_MALFORMED_FACETED_BREP = 4
};

#endif /* csmstmodeler_h */

