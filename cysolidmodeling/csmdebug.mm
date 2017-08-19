//
//  csmdebug.c
//  cysolidmodeling
//
//  Created by Manuel Fernandez on 23/7/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmdebug.inl"

extern "C"
{
    #include "csmmath.inl"
}

#include <basicSystem/bsassert.h>
#include <basicSystem/bsmaterial.h>
#include "cypestr.h"
#include "a_punter.h"

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

static CYBOOL g_Draw_plane = FALSO;
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
static FILE *g_output_file = NULL;

// --------------------------------------------------------------------------------

void csmdebug_set_enabled_by_code(CYBOOL enabled)
{
    if (enabled == CIERTO)
        i_DEBUG_IS_DISABLED_BY_CODE = 0;
    else
        i_DEBUG_IS_DISABLED_BY_CODE = 1;
}

// --------------------------------------------------------------------------------

CYBOOL csmdebug_debug_enabled(void)
{
    if (i_DEBUG_IS_DISABLED_BY_CODE == 1)
        return FALSO;
    else
        return ES_CIERTO(i_DEBUG_SCREEN == 1 || g_output_file != NULL);
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
        if (i_DEBUG_SCREEN == CIERTO) fprintf(stdout, "%s", buffer);\
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
    if (csmdebug_debug_enabled() == CIERTO)
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
    if (csmdebug_debug_enabled() == CIERTO)
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
    if (csmdebug_debug_enabled() == CIERTO)
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
    if (i_DEBUG_IS_DISABLED_BY_CODE == FALSO && i_DEBUG_VISUAL == CIERTO && g_func_show_viewer != NULL)
    {
        bsassert(g_Viewer != NULL);
        g_func_show_viewer(g_Viewer);
    }
}

// --------------------------------------------------------------------------------

void csmdebug_clear_debug_points(void)
{
    g_Draw_plane = FALSO;
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
    
    cypestr_destruye(description);
}

// --------------------------------------------------------------------------------

void csmdebug_set_plane(double A, double B, double C, double D)
{
    g_Draw_plane = CIERTO;
    g_A = A;
    g_B = B;
    g_C = C;
    g_D = D;
}

// --------------------------------------------------------------------------------

void csmdebug_draw_debug_info(struct bsgraphics2_t *graphics)
{
    unsigned long i;
    
    for (i = 0; i < g_no_debug_points; i++)
    {
        bsgraphics2_escr_punto3D(graphics, g_Debug_points[i].x, g_Debug_points[i].y, g_Debug_points[i].z);
        
        bsgraphics2_append_desplazamiento_3D(graphics, g_Debug_points[i].x, g_Debug_points[i].y, g_Debug_points[i].z);
        bsgraphics2_append_ejes_plano_pantalla(graphics);

        bsgraphics2_escr_texto_mts(
                    graphics,
                    g_Debug_points[i].text, 0., 0., 1., 0.,
                    BSGRAPHICS2_JUSTIFICACION_CEN_CEN,
                    BSGRAPHICS2_ESTILO_NORMAL,
                    0.05);
        
        bsgraphics2_desapila_transformacion(graphics);
        bsgraphics2_desapila_transformacion(graphics);        
    }
    
    if (g_Draw_plane == CIERTO)
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
    }
}



