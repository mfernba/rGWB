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
    
#ifdef __STANDALONE_DISTRIBUTABLE

DLL_RGWB enum csmstlimporter_result_t csmstlimporter_did_read_binary_stl(const char *file_path, struct csmsolid_t **solid);

DLL_RGWB enum csmstlimporter_result_t csmstlimporter_did_read_ascii_stl(const char *file_path, struct csmsolid_t **solid);

#else

DLL_RGWB enum csmstlimporter_result_t csmstlimporter_did_read_binary_stl_u(const wchar_t *file_path, struct csmsolid_t **solid);

DLL_RGWB void csmstlimporter_convert_binary_stl_to_text_u(const wchar_t *file_path_in, const wchar_t *file_path_out);

#endif
    
#ifdef __cplusplus
}
#endif

