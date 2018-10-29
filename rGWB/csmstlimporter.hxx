//
//  csmstlimporter.hxx
//  rGWB
//
//  Created by Manuel Fernandez on 29/10/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#ifndef csmstlimporter_hxx
#define csmstlimporter_hxx

enum csmstlimporter_result_t
{
    CSMSTLIMPORTER_RESULT_OK = 0,
    CSMSTLIMPORTER_RESULT_FILE_NOT_FOUND = 1,
    CSMSTLIMPORTER_RESULT_INCORRECT_FORMAT = 2,
    CSMSTLIMPORTER_RESULT_EMPTY_SOLID = 3,
    CSMSTLIMPORTER_RESULT_NON_MANIFOLD_FACETED_BREP = 4,
    CSMSTLIMPORTER_RESULT_MALFORMED_FACETED_BREP = 5,
    CSMSTLIMPORTER_RESULT_INCONSISTENT_INNER_LOOP_ORIENTATION = 6,
    CSMSTLIMPORTER_RESULT_INCONSISTENT_IMPROPER_INTERSECTIONS = 7
};

#endif /* csmstlimporter_hxx */
