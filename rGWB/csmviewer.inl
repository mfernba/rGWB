//
//  smtviewer.h
//  SolidModelingTester
//
//  Created by Manuel Fernandez on 23/7/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#ifdef __cplusplus
extern "C" {
#endif
    
struct csmviewer_t *csmviewer_new(void);

void csmviewer_free(struct csmviewer_t **viewer);


void csmviewer_set_parameters(struct csmviewer_t *viewer, struct csmsolid_t *solid1, struct csmsolid_t *solid2);

void csmviewer_set_results(struct csmviewer_t *viewer, struct csmsolid_t *solid1, struct csmsolid_t *solid2);

    
void csmviewer_show(struct csmviewer_t *viewer);
    
void csmviewer_show_face(struct csmviewer_t *viewer, struct csmface_t *face1, struct csmface_t *face2);
    
#ifdef __cplusplus
}
#endif
