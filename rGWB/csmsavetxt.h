//
//  csmsavetxt.h
//  rGWB
//
//  Created by Manuel Fernández on 1/11/18.
//  Copyright © 2018 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

DLL_RGWB CONSTRUCTOR(struct csmsave_t *, csmsavetxt_new_file_writer, (const char *file_path));

DLL_RGWB CONSTRUCTOR(struct csmsave_t *, csmsavetxt_new_file_reader, (const char *file_path));

#ifdef __cplusplus
}
#endif
