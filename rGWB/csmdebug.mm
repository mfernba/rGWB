//
//  csmdebug.c
//  rGWB
//
//  Created by Manuel Fernandez on 23/7/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmdebug.inl"

extern "C"
{
    #include "csmmath.inl"
    #include "csmstring.inl"
}

#include <basicSystem/bsassert.h>
#include <basicSystem/bsmaterial.h>

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <basicGraphics/bsgraphics2.h>
#include <basicGraphics/bsgraphics2.tlh>

#define i_MAX_NUM_CONTEXTS 100
#define i_MAX_LENGTH_CONTEXT_NAME 256

static char i_CONTEXT_STACK[i_MAX_NUM_CONTEXTS][i_MAX_LENGTH_CONTEXT_NAME];
static unsigned long i_NO_STACKED_CONTEXTS = 0;
static int g_INITIALIZED = 0;

#define i_MAX_NUM_POINTS 1000
#define i_MAX_DESCRIPTION_LENGHT 256

struct i_debug_point_t
{
    double x, y, z;
    char text[i_MAX_DESCRIPTION_LENGHT];
};

static struct i_debug_point_t g_Debug_points[i_MAX_NUM_POINTS];
static unsigned long g_no_debug_points = 0;

static CSMBOOL g_Draw_plane = CSMFALSE;
static double g_A = 0., g_B = 0., g_C = 0., g_D = 0.;

typedef void (*i_FPtr_show_viewer)(struct csmviewer_t *);
typedef void (*i_FPtr_func_set_parameters)(struct csmviewer_t *, struct csmsolid_t *solid1, struct csmsolid_t *solid2);

static struct csmviewer_t *g_Viewer = NULL;
static i_FPtr_show_viewer g_func_show_viewer = NULL;
static i_FPtr_func_set_parameters g_func_set_viewer_parameters = NULL;
static i_FPtr_func_set_parameters g_func_set_viewer_results = NULL;

static int i_DEBUG_IS_DISABLED_BY_CODE = 0;
static int i_DEBUG_SCREEN = 1;
static int i_DEBUG_VISUAL = 1;
static int i_DEBUG_FILE = 1;

static int i_DEBUG_PRINT_SOLID_BLOCKED = 0;

static CSMBOOL i_TREAT_IMPROPER_SOLID_OPERATIONS_AS_ERRORS = CSMFALSE;

static FILE *g_output_file = NULL;

struct i_inters_sector_t
{
    double x, y, z;
    
    CSMBOOL with_intersection_line;
    double Wx_inters, Wy_inters, Wz_inters;

    double Ux1, Uy1, Uz1, Vx1, Vy1, Vz1;
    double Ux2, Uy2, Uz2, Vx2, Vy2, Vz2;
};

static int g_Draw_inters_sector = 0;
struct i_inters_sector_t g_inters_sector;

// --------------------------------------------------------------------------------

void csmdebug_set_treat_improper_solid_operations_as_errors(CSMBOOL value)
{
    i_TREAT_IMPROPER_SOLID_OPERATIONS_AS_ERRORS = value;
}

// --------------------------------------------------------------------------------

CSMBOOL csmdebug_get_treat_improper_solid_operations_as_errors(void)
{
    return i_TREAT_IMPROPER_SOLID_OPERATIONS_AS_ERRORS;
}

// --------------------------------------------------------------------------------

void csmdebug_set_enabled_by_code(CSMBOOL enabled)
{
    if (enabled == CSMTRUE)
        i_DEBUG_IS_DISABLED_BY_CODE = 0;
    else
        i_DEBUG_IS_DISABLED_BY_CODE = 1;
}

// --------------------------------------------------------------------------------

CSMBOOL csmdebug_debug_enabled(void)
{
    if (i_DEBUG_IS_DISABLED_BY_CODE == 1)
        return CSMFALSE;
    else
        return IS_TRUE(i_DEBUG_SCREEN == 1 || g_output_file != NULL);
}

// --------------------------------------------------------------------------------

static void i_init(void)
{
    unsigned long i;
    
    for (i = 0; i < i_MAX_NUM_CONTEXTS; i++)
        memset(i_CONTEXT_STACK[i], '\0', i_MAX_LENGTH_CONTEXT_NAME);
    
    for (i = 0; i < i_MAX_NUM_POINTS; i++)
    {
        g_Debug_points[i].x = 0.;
        g_Debug_points[i].y = 0.;
        g_Debug_points[i].z = 0.;
        g_Debug_points[i].text[0] = '\0';
    }
}

// --------------------------------------------------------------------------------

#define i_print_format(format)\
(\
    {\
        int res;\
        char buffer[10000];\
        va_list argptr;\
        \
        va_start(argptr, format);\
        res = vsnprintf(buffer, sizeof(buffer) - 1, format, argptr);\
        bsassert(res >= 0);\
        buffer[sizeof(buffer) - 1] = '\0';\
        va_end(argptr);\
        \
        if (i_DEBUG_SCREEN == CSMTRUE) fprintf(stdout, "%s", buffer);\
        if (g_output_file != NULL) fprintf(g_output_file, "%s", buffer);\
    }\
)

// --------------------------------------------------------------------------------

static void i_print_tab_level(unsigned long no_tabs)
{
    unsigned long i;
    
    bsassert(no_tabs > 0);
    
    for (i = 0; i< no_tabs - 1; i++)
    {
        fprintf(stdout, "  ");
        
        if (g_output_file != NULL)
            fprintf(g_output_file, "  ");
    }
}

// --------------------------------------------------------------------------------

void csmdebug_begin_context(const char *context)
{
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        bsassert(i_NO_STACKED_CONTEXTS < i_MAX_NUM_CONTEXTS);
    
        if (i_NO_STACKED_CONTEXTS == 0 && g_INITIALIZED == 0)
        {
            i_init();
            g_INITIALIZED = 1;
        }
    
        strlcpy(i_CONTEXT_STACK[++i_NO_STACKED_CONTEXTS], context, i_MAX_LENGTH_CONTEXT_NAME);
        csmdebug_print_debug_info("BEGIN CONTEXT: %s\n", context);
    }
}

// --------------------------------------------------------------------------------

void csmdebug_end_context(void)
{
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        bsassert(i_NO_STACKED_CONTEXTS > 0);
        csmdebug_print_debug_info("END CONTEXT\n", NULL);
    
        strlcpy(i_CONTEXT_STACK[i_NO_STACKED_CONTEXTS], "", i_MAX_LENGTH_CONTEXT_NAME);
        i_NO_STACKED_CONTEXTS--;
    }
}

// --------------------------------------------------------------------------------

void csmdebug_print_debug_info(const char *format, ...)
{
    if (csmdebug_debug_enabled() == CSMTRUE)
    {
        if (i_NO_STACKED_CONTEXTS > 0)
            i_print_tab_level(i_NO_STACKED_CONTEXTS);
        
        i_print_format(format);
        fflush(stdout);
        
        if (g_output_file != NULL)
            fflush(g_output_file);
    }
}

// --------------------------------------------------------------------------------

void csmdebug_set_ouput_file(const char *file_path)
{
    if (i_DEBUG_FILE == 1)
    {
        g_output_file = fopen(file_path, "wt");
        bsassert_not_null(g_output_file);
    }
}

// --------------------------------------------------------------------------------

void csmdebug_close_output_file(void)
{
    if (g_output_file != NULL)
    {
        fflush(g_output_file);
        fclose(g_output_file);
        
        g_output_file = NULL;
    }
}

// --------------------------------------------------------------------------------

CSMBOOL csmdebug_is_print_solid_unblocked(void)
{
    return !i_DEBUG_PRINT_SOLID_BLOCKED;
}
    
// --------------------------------------------------------------------------------

void csmdebug_block_print_solid(void)
{
    i_DEBUG_PRINT_SOLID_BLOCKED = CSMTRUE;
}
    
// --------------------------------------------------------------------------------

void csmdebug_unblock_print_solid(void)
{
    i_DEBUG_PRINT_SOLID_BLOCKED = CSMFALSE;
}

// --------------------------------------------------------------------------------

void csmdebug_set_viewer(
                    struct csmviewer_t *viewer,
                    void (*func_show_viewer)(struct csmviewer_t *),
                    void (*func_set_parameters)(struct csmviewer_t *, struct csmsolid_t *solid1, struct csmsolid_t *solid2),
                    void (*func_set_results)(struct csmviewer_t *, struct csmsolid_t *solid1, struct csmsolid_t *solid2))
{
    g_Viewer = viewer;
    g_func_show_viewer = func_show_viewer;
    g_func_set_viewer_parameters = func_set_parameters;
    g_func_set_viewer_results = func_set_results;
}

// --------------------------------------------------------------------------------

void csmdebug_set_viewer_parameters(struct csmsolid_t *solid1, struct csmsolid_t *solid2)
{
    if (g_func_set_viewer_parameters != NULL)
        g_func_set_viewer_parameters(g_Viewer, solid1, solid2);
}

// --------------------------------------------------------------------------------

void csmdebug_set_viewer_results(struct csmsolid_t *solid1, struct csmsolid_t *solid2)
{
    if (g_func_set_viewer_results != NULL)
        g_func_set_viewer_results(g_Viewer, solid1, solid2);
}

// --------------------------------------------------------------------------------

void csmdebug_show_viewer(void)
{
    if (i_DEBUG_IS_DISABLED_BY_CODE == CSMFALSE && i_DEBUG_VISUAL == CSMTRUE && g_func_show_viewer != NULL)
    {
        bsassert(g_Viewer != NULL);
        g_func_show_viewer(g_Viewer);
    }
}

// --------------------------------------------------------------------------------

void csmdebug_clear_debug_points(void)
{
    g_Draw_plane = CSMFALSE;
    g_no_debug_points = 0;
}

// --------------------------------------------------------------------------------

void csmdebug_append_debug_point(double x, double y, double z, char **description)
{
    bsassert_not_null(description);
    
    if (i_DEBUG_VISUAL == 1)
    {
        bsassert(g_no_debug_points < i_MAX_NUM_POINTS);
        
        g_Debug_points[g_no_debug_points].x = x;
        g_Debug_points[g_no_debug_points].y = y;
        g_Debug_points[g_no_debug_points].z = z;
        strlcpy(g_Debug_points[g_no_debug_points].text, *description, i_MAX_DESCRIPTION_LENGHT);
        
        g_no_debug_points++;
    }
    
    csmstring_free(description);
}

// --------------------------------------------------------------------------------

void csmdebug_clear_plane(void)
{
    g_Draw_plane = CSMFALSE;
}

// --------------------------------------------------------------------------------

void csmdebug_set_plane(double A, double B, double C, double D)
{
    g_Draw_plane = CSMTRUE;
    g_A = A;
    g_B = B;
    g_C = C;
    g_D = D;
}

// --------------------------------------------------------------------------------

void csmdebug_clear_inters_sector(void)
{
    g_Draw_inters_sector = 0;
}

// --------------------------------------------------------------------------------

void csmdebug_set_inters_sector(
                    double x, double y, double z,
                    CSMBOOL with_intersection_line, double Wx_inters, double Wy_inters, double Wz_inters,
                    double Ux1, double Uy1, double Uz1, double Vx1, double Vy1, double Vz1,
                    double Ux2, double Uy2, double Uz2, double Vx2, double Vy2, double Vz2)
{
    g_Draw_inters_sector = 1;
    
    g_inters_sector.x = x;
    g_inters_sector.y = y;
    g_inters_sector.z = z;
    
    g_inters_sector.with_intersection_line = with_intersection_line;
    g_inters_sector.Wx_inters = Wx_inters;
    g_inters_sector.Wy_inters = Wy_inters;
    g_inters_sector.Wz_inters = Wz_inters;
    
    g_inters_sector.Ux1 = Ux1;
    g_inters_sector.Uy1 = Uy1;
    g_inters_sector.Uz1 = Uz1;
    
    g_inters_sector.Vx1 = Vx1;
    g_inters_sector.Vy1 = Vy1;
    g_inters_sector.Vz1 = Vz1;
    
    g_inters_sector.Ux2 = Ux2;
    g_inters_sector.Uy2 = Uy2;
    g_inters_sector.Uz2 = Uz2;
    
    g_inters_sector.Vx2 = Vx2;
    g_inters_sector.Vy2 = Vy2;
    g_inters_sector.Vz2 = Vz2;
}

// --------------------------------------------------------------------------------

static void i_draw_inters_sector(const struct i_inters_sector_t *sector, struct bsgraphics2_t *graphics)
{
    struct bsmaterial_t *mat1, *mat2;
    
    bsassert_not_null(sector);
    
    bsgraphics2_escr_punto3D(graphics, sector->x, sector->y, sector->z);
    
    mat1 = bsmaterial_crea_rgb(1., 0., 0.);
    bsgraphics2_escr_color(graphics, mat1);
    {
        bsgraphics2_escr_triangulo3D(
                graphics,
                sector->x, sector->y, sector->z,
                sector->x + sector->Ux1, sector->y + sector->Uy1, sector->z + sector->Uz1,
                sector->x + sector->Vx1, sector->y + sector->Vy1, sector->z + sector->Vz1);
    }
    bsmaterial_destruye(&mat1);
    
    mat2 = bsmaterial_crea_rgb(1., 1., 0.);
    bsgraphics2_escr_color(graphics, mat2);
    {
        bsgraphics2_escr_triangulo3D(
                graphics,
                sector->x, sector->y, sector->z,
                sector->x + sector->Ux2, sector->y + sector->Uy2, sector->z + sector->Uz2,
                sector->x + sector->Vx2, sector->y + sector->Vy2, sector->z + sector->Vz2);
    }
    bsmaterial_destruye(&mat2);
    
    if (sector->with_intersection_line == CSMTRUE)
    {
        struct bsmaterial_t *mat3;
        
        mat3 = bsmaterial_crea_rgb(0.5, 0.5, 0.5);
        bsgraphics2_escr_color(graphics, mat3);

        bsgraphics2_escr_linea3D(
                graphics,
                sector->x, sector->y, sector->z,
                sector->x + sector->Wx_inters, sector->y + sector->Wy_inters, sector->z + sector->Wz_inters);
        
        bsmaterial_destruye(&mat3);
    }
}

// --------------------------------------------------------------------------------

void csmdebug_draw_debug_info(struct bsgraphics2_t *graphics)
{
    struct bsmaterial_t *debug_point_material;
    unsigned long i;
    
    debug_point_material = bsmaterial_crea_rgb(1., 0., 0.);

    bsgraphics2_escr_color(graphics, debug_point_material);
    
    //bsgraphics2_escr_punto3D(graphics, 0.0815685, 0.0565685, 0.3);
    //bsgraphics2_escr_punto3D(graphics, 0.0815685, 0.0565685, 0.3 + 1.e-3);
    
    //bsgraphics2_escr_punto3D(graphics, -0.0282, -0.0118,  0.55);
    //bsgraphics2_escr_punto3D(graphics, -0.0282,  0.0118,  0.55);
    //bsgraphics2_escr_punto3D(graphics, -0.006, -0.050,  0.550);
    //bsgraphics2_escr_punto3D(graphics, -0.035, -0.000,  0.550);
    
    /*
     bsgraphics2_escr_punto3D(graphics, -0.035, -2.46e-11,   0.1);
	bsgraphics2_escr_punto3D(graphics, -0.0324, -0.00264,   0.1);
	bsgraphics2_escr_punto3D(graphics, -0.0324, 0.00264,   0.1);
	bsgraphics2_escr_punto3D(graphics, -0.00144, -0.0336,  0.05);
	bsgraphics2_escr_punto3D(graphics, -0.00144, -0.0336,   0.1);
	bsgraphics2_escr_punto3D(graphics, -0.00144, 0.0336,  0.05);
	bsgraphics2_escr_punto3D(graphics, -0.00144, 0.0336,   0.1);
	bsgraphics2_escr_punto3D(graphics, 0.0124, -0.038,  0.05);
	bsgraphics2_escr_punto3D(graphics, 0.0124, 0.038,  0.05);
	bsgraphics2_escr_punto3D(graphics, 0.025, -0.06,   0.1);
	bsgraphics2_escr_punto3D(graphics, 0.025,  0.06,   0.1);
	bsgraphics2_escr_punto3D(graphics, 0.04,     0,  0.05);
	bsgraphics2_escr_punto3D(graphics, 0.085,     0,   0.1);
*/
    
    /*
    bsgraphics2_escr_punto3D(graphics, -0.0324, -0.00264,   0.1);
    bsgraphics2_escr_punto3D(graphics, -0.0324,  0.00264,   0.1);
    
    bsgraphics2_escr_punto3D(graphics, -0.035, -0.000,  0.100);
    bsgraphics2_escr_punto3D(graphics, -0.032, -0.003,  0.100);
    bsgraphics2_escr_punto3D(graphics, -0.032, -0.003,  0.100);
    bsgraphics2_escr_punto3D(graphics, -0.035, -0.000,  0.100);
    bsgraphics2_escr_punto3D(graphics, -0.032,  0.003,  0.100);
    bsgraphics2_escr_punto3D(graphics, -0.032,  0.003,  0.100);
    */

    if (g_Draw_inters_sector)
        i_draw_inters_sector(&g_inters_sector, graphics);

    for (i = 0; i < g_no_debug_points; i++)
    {
        enum bsgraphics2_justificacion_t justificacion;
        
        bsgraphics2_escr_color(graphics, debug_point_material);
        bsgraphics2_escr_punto3D(graphics, g_Debug_points[i].x, g_Debug_points[i].y, g_Debug_points[i].z);
        
        //if (fabs(fabs(g_Debug_points[i].z) - 0.05) < 1.e-3)
        {
            bsgraphics2_append_desplazamiento_3D(graphics, g_Debug_points[i].x, g_Debug_points[i].y, g_Debug_points[i].z);
            bsgraphics2_append_ejes_plano_pantalla(graphics);
            
            (i % 2 == 0) ? justificacion = BSGRAPHICS2_JUSTIFICACION_SUP_CEN: justificacion = BSGRAPHICS2_JUSTIFICACION_INF_CEN;

            bsgraphics2_escr_texto_mts(
                    graphics,
                    g_Debug_points[i].text, 0., 0., 1., 0.,
                    justificacion,
                    BSGRAPHICS2_ESTILO_NORMAL,
                    0.002);
        
            bsgraphics2_desapila_transformacion(graphics);
            bsgraphics2_desapila_transformacion(graphics);
        }
    }
    
    /*if (g_Draw_plane == CSMTRUE)
    {
        double Xo, Yo, Zo, Ux, Uy, Uz, Vx, Vy, Vz;
        double desp;
        double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
        struct bsmaterial_t *material;
        
        csmmath_plane_axis_from_implicit_plane_equation(g_A, g_B, g_C, g_D, &Xo, &Yo, &Zo, &Ux, &Uy, &Uz, &Vx, &Vy, &Vz);
        
        desp = 2;
        
        csmmath_move_point(Xo, Yo, Zo, Ux, Uy, Uz, -desp, &x1, &y1, &z1);
        csmmath_move_point(x1, y1, z1, Vx, Vy, Vz, -desp, &x1, &y1, &z1);

        csmmath_move_point(Xo, Yo, Zo, Ux, Uy, Uz,  desp, &x2, &y2, &z2);
        csmmath_move_point(x2, y2, z2, Vx, Vy, Vz, -desp, &x2, &y2, &z2);

        csmmath_move_point(Xo, Yo, Zo, Ux, Uy, Uz,  desp, &x3, &y3, &z3);
        csmmath_move_point(x3, y3, z3, Vx, Vy, Vz,  desp, &x3, &y3, &z3);

        csmmath_move_point(Xo, Yo, Zo, Ux, Uy, Uz, -desp, &x4, &y4, &z4);
        csmmath_move_point(x4, y4, z4, Vx, Vy, Vz,  desp, &x4, &y4, &z4);
        
        material = bsmaterial_crea_rgba(0.5, 0.5, 0.5, 0.1);
        bsgraphics2_escr_color(graphics, material);
        
        bsgraphics2_escr_cuadrilatero3D_ex(
                        graphics,
                        x1, y1, z1, true, g_A, g_B, g_C,
                        x2, y2, z2, true, g_A, g_B, g_C,
                        x3, y3, z3, true, g_A, g_B, g_C,
                        x4, y4, z4, true, g_A, g_B, g_C);
        
        bsmaterial_destruye(&material);
    }*/
    
    bsmaterial_destruye(&debug_point_material);
}



