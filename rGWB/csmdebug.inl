//
//  csmdebug.h
//  rGWB
//
//  Created by Manuel Fernandez on 23/7/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmfwddecl.hxx"

#ifdef __cplusplus
extern "C" {
#endif

void csmdebug_set_enabled_by_code(CSMBOOL enabled);
    
CSMBOOL csmdebug_debug_enabled(void);

    
void csmdebug_begin_context(const char *context);

void csmdebug_end_context(void);

void csmdebug_print_debug_info(const char *format, ...);


// Textual debug...

void csmdebug_set_ouput_file(const char *file_path);
    
void csmdebug_close_output_file(void);


CSMBOOL csmdebug_is_print_solid_unblocked(void);
    
void csmdebug_block_print_solid(void);
    
void csmdebug_unblock_print_solid(void);
    

// Graphical debug...

void csmdebug_set_viewer(
                    struct csmviewer_t *viewer,
                    void (*func_show_viewer)(struct csmviewer_t *),
                    void (*func_set_parameters)(struct csmviewer_t *, struct csmsolid_t *solid1, struct csmsolid_t *solid2),
                    void (*func_set_results)(struct csmviewer_t *, struct csmsolid_t *solid1, struct csmsolid_t *solid2));

void csmdebug_set_viewer_parameters(struct csmsolid_t *solid1, struct csmsolid_t *solid2);
    
void csmdebug_set_viewer_results(struct csmsolid_t *solid_res1, struct csmsolid_t *solid2_res2);
    
void csmdebug_show_viewer(void);


void csmdebug_clear_debug_points(void);
void csmdebug_append_debug_point(double x, double y, double z, char **description);

void csmdebug_clear_plane(void);
void csmdebug_set_plane(double A, double B, double C, double D);

void csmdebug_clear_inters_sector(void);

void csmdebug_set_inters_sector(
                    double x, double y, double z,
                    CSMBOOL with_intersection_line, double Wx_inters, double Wy_inters, double Wz_inters,
                    double Ux1, double Uy1, double Uz1, double Vx1, double Vy1, double Vz1,
                    double Ux2, double Uy2, double Uz2, double Vx2, double Vy2, double Vz2);
                                
void csmdebug_draw_debug_info(struct bsgraphics2_t *graphics);


    
#ifdef __cplusplus
}
#endif
