//
//  csmstlimporter.h
//  rGWB
//
//  Created by Manuel Fernandez on 29/10/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif
    
DLL_RGWB enum csmstlimporter_result_t csmstlimporter_did_read_binary_stl(const char *file_path, struct csmsolid_t **solid);

DLL_RGWB enum csmstlimporter_result_t csmstlimporter_did_read_ascii_stl(const char *file_path, struct csmsolid_t **solid);
    
#ifdef __cplusplus
}
#endif

