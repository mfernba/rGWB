//
//  csmshape2d.c
//  rGWB
//
//  Created by Manuel Fernández on 19/11/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmshape2d.h"
#include "csmshape2d.inl"

#include "csmassert.inl"
#include "csmmem.inl"
#include "csmmath.inl"
#include "csmgeom.inl"
#include "csmarrayc.inl"
#include "csmArrPoint2D.h"
#include "csmArrPoint3D.h"

#include "basicGraphics/bsgraphics2.h"
#include <OpenGL/OpenGL.h>
#include <GLUT/GLUT.h>

struct i_polygon_t
{
	csmArrPoint2D *points;
    csmArrPoint3D *normals;
};

struct csmshape2d_t
{
	csmArrayStruct(i_polygon_t)	*polygons;
};

struct i_tesselator_vertex_t
{
    double x, y;
    double Nx, Ny, Nz;
    CSMBOOL is_vertex_on_edge;
};

struct i_tesselator_data_t
{
    CSMBOOL current_vertex_on_edge;
	csmArrayStruct(i_tesselator_vertex_t) *vertexs;
};

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_polygon_t *, i_new_polygon, (csmArrPoint2D **points, csmArrPoint3D **normals))
{
	struct i_polygon_t *polygon;
	
	polygon = MALLOC(struct i_polygon_t);
    
	polygon->points = ASIGNA_PUNTERO_PP_NO_NULL(points, csmArrPoint2D);
    polygon->normals = ASIGNA_PUNTERO_PP_NO_NULL(normals, csmArrPoint3D);
	
	return polygon;
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_polygon_t *, i_copy_polygon, (const struct i_polygon_t *polygon))
{
    csmArrPoint2D *points;
    csmArrPoint3D *normals;
    
    assert_no_null(polygon);
    
    points = csmArrPoint2D_copy(polygon->points);
    normals = csmArrPoint3D_copy(polygon->normals);
	
	return i_new_polygon(&points, &normals);
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_polygon_t *, i_new_empty_polygon, (void))
{
	csmArrPoint2D *points;
    csmArrPoint3D *normals;
	
	points = csmArrPoint2D_new(0);
    normals = csmArrPoint3D_new(0);
    
	return i_new_polygon(&points, &normals);
}

// --------------------------------------------------------------------------------

static void i_free_polygon(struct i_polygon_t **polygon)
{
	assert_no_null(polygon);
	assert_no_null(*polygon);
	
	csmArrPoint2D_free(&(*polygon)->points);
    csmArrPoint3D_free(&(*polygon)->normals);
	
	FREE_PP(polygon, struct i_polygon_t);
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmshape2d_t *, i_crea, (csmArrayStruct(i_polygon_t) **polygons))
{
	struct csmshape2d_t *shape2d;
	
	shape2d = MALLOC(struct csmshape2d_t);
	shape2d->polygons = ASIGNA_PUNTERO_PP_NO_NULL(polygons, csmArrayStruct(i_polygon_t));
	
	return shape2d;
}

// --------------------------------------------------------------------------------

struct csmshape2d_t *csmshape2d_new(void)
{
	csmArrayStruct(i_polygon_t) *polygons;
	
	polygons = csmarrayc_new_st_array(0, i_polygon_t);
	return i_crea(&polygons);
}

// --------------------------------------------------------------------------------

struct csmshape2d_t *csmshape2d_copy(const struct csmshape2d_t *shape2d)
{
	csmArrayStruct(i_polygon_t) *polygons;
	
    assert_no_null(shape2d);
    
    polygons = csmarrayc_copy_st_array(shape2d->polygons, i_polygon_t, i_copy_polygon);
    
	return i_crea(&polygons);
}

// --------------------------------------------------------------------------------

void csmshape2d_free(struct csmshape2d_t **shape2d)
{
	assert_no_null(shape2d);
	assert_no_null(*shape2d);
	
	csmarrayc_free_st(&(*shape2d)->polygons, i_polygon_t, i_free_polygon);
	
	FREE_PP(shape2d, struct csmshape2d_t);
}

// --------------------------------------------------------------------------------

unsigned long csmshape2d_polygon_count(const struct csmshape2d_t *shape2d)
{
	assert_no_null(shape2d);
	return csmarrayc_count_st(shape2d->polygons, i_polygon_t);
}

// --------------------------------------------------------------------------------

unsigned long csmshape2d_point_polygon_count(const struct csmshape2d_t *shape2d, unsigned long idx_polygon)
{
	const struct i_polygon_t *polygon;
	
	assert_no_null(shape2d);
	
	polygon = csmarrayc_get_const_st(shape2d->polygons, idx_polygon, i_polygon_t);
	assert_no_null(polygon);
	
	return csmArrPoint2D_count(polygon->points);
}

// --------------------------------------------------------------------------------

void csmshape2d_new_polygon(struct csmshape2d_t *shape2d, unsigned long *idx_new_polygon)
{
	struct i_polygon_t *polygon;
	
	assert_no_null(shape2d);
	assert_no_null(idx_new_polygon);
	
	polygon = i_new_empty_polygon();
	csmarrayc_append_element_st(shape2d->polygons, polygon, i_polygon_t);
	
	*idx_new_polygon = csmarrayc_count_st(shape2d->polygons, i_polygon_t) - 1;
}

// --------------------------------------------------------------------------------

void csmshape2d_append_point_to_polygon(struct csmshape2d_t *shape2d, unsigned long idx_polygon, double x, double y)
{
	struct i_polygon_t *polygon;
	
	assert_no_null(shape2d);
	
	polygon = csmarrayc_get_st(shape2d->polygons, idx_polygon, i_polygon_t);
	assert_no_null(polygon);
	
	csmArrPoint2D_append(polygon->points, x, y);
    csmArrPoint3D_append(polygon->normals, 0., 0., 0.);
}

// --------------------------------------------------------------------------------

void csmshape2d_append_new_polygon_with_points(struct csmshape2d_t *shape2d, csmArrPoint2D **points)
{
    unsigned long i, num_points;
    csmArrPoint3D *normals;
	struct i_polygon_t *polygon;
	
	assert_no_null(shape2d);
    assert_no_null(points);
    num_points = csmArrPoint2D_count(*points);
    assert(num_points > 0);
    
    normals = csmArrPoint3D_new(num_points);
    
    for (i = 0; i < num_points; i++)
        csmArrPoint3D_set(normals, i, 0., 0., 0.);
    
	polygon = i_new_polygon(points, &normals);
	csmarrayc_append_element_st(shape2d->polygons, polygon, i_polygon_t);
}

// --------------------------------------------------------------------------------

void csmshape2d_append_new_polygon_with_points_and_normals(struct csmshape2d_t *shape2d, csmArrPoint2D **points, csmArrPoint3D **normals)
{
    struct i_polygon_t *polygon;
	
	assert_no_null(shape2d);
    assert_no_null(points);
    assert_no_null(normals);
    assert(csmArrPoint2D_count(*points) == csmArrPoint3D_count(*normals));
    
    polygon = i_new_polygon(points, normals);
	csmarrayc_append_element_st(shape2d->polygons, polygon, i_polygon_t);
}

// --------------------------------------------------------------------------------

void csmshape2d_point_polygon_coords(
						const struct csmshape2d_t *shape2d,
						unsigned long idx_polygon, unsigned long idx_point,
						double *x, double *y)
{
	const struct i_polygon_t *polygon;
	
	assert_no_null(shape2d);
	
	polygon = csmarrayc_get_const_st(shape2d->polygons, idx_polygon, i_polygon_t);
	assert_no_null(polygon);
	
	csmArrPoint2D_get(polygon->points, idx_point, x, y);
}

// --------------------------------------------------------------------------------

CSMBOOL csmshape2d_polygon_is_hole(const struct csmshape2d_t *shape2d, unsigned long idx_polygon)
{
	const struct i_polygon_t *polygon;
    double area_poligono;
	
	assert_no_null(shape2d);
	
	polygon = csmarrayc_get_const_st(shape2d->polygons, idx_polygon, i_polygon_t);
	assert_no_null(polygon);
    
    area_poligono = csmArrPoint2D_area(polygon->points);
    
    if (area_poligono < 0.)
        return CSMTRUE;
    else
        return CSMFALSE;
}

// --------------------------------------------------------------------------------

void csmshape2d_move(struct csmshape2d_t *shape2d, double dx, double dy)
{
	unsigned long i, num_polygons;
    
    assert_no_null(shape2d);
	
	num_polygons = csmarrayc_count_st(shape2d->polygons, i_polygon_t);
	assert(num_polygons > 0);
	
	for (i = 0; i < num_polygons; i++)
	{
        const struct i_polygon_t *polygon;
        
        polygon = csmarrayc_get_const_st(shape2d->polygons, i, i_polygon_t);
        assert_no_null(polygon);
        
        csmArrPoint2D_move(polygon->points, dx, dy);
	}
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_tesselator_vertex_t *, i_new_tesselator_vertex, (double x, double y, double Nx, double Ny, double Nz, CSMBOOL is_vertex_on_edge))
{
    struct i_tesselator_vertex_t *tesselator_vertex;
    
    tesselator_vertex = MALLOC(struct i_tesselator_vertex_t);
    
    tesselator_vertex->x = x;
    tesselator_vertex->y = y;
    
    tesselator_vertex->Nx = Nx;
    tesselator_vertex->Ny = Ny;
    tesselator_vertex->Nz = Nz;
    
    tesselator_vertex->is_vertex_on_edge = is_vertex_on_edge;
    
    return tesselator_vertex;
}

// --------------------------------------------------------------------------------

static void i_free_tesselator_vertex(struct i_tesselator_vertex_t **tesselator_vertex)
{
    FREE_PP(tesselator_vertex, struct i_tesselator_vertex_t);
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_tesselator_data_t *, i_new_tesselator_data, (
                        CSMBOOL current_vertex_on_edge,
                        csmArrayStruct(i_tesselator_vertex_t) **vertexs))
{
    struct i_tesselator_data_t *tesselator_data;
    
    tesselator_data = MALLOC(struct i_tesselator_data_t);
    
    tesselator_data->current_vertex_on_edge = current_vertex_on_edge;
    tesselator_data->vertexs = ASIGNA_PUNTERO_PP_NO_NULL(vertexs, csmArrayStruct(i_tesselator_vertex_t));
    
    return tesselator_data;
}

// --------------------------------------------------------------------------------

static void i_free_tesselator_data(struct i_tesselator_data_t **tesselator_data)
{
    assert_no_null(tesselator_data);
    assert_no_null(*tesselator_data);
    
    csmarrayc_free_st(&(*tesselator_data)->vertexs, i_tesselator_vertex_t, i_free_tesselator_vertex);
    
    FREE_PP(tesselator_data, struct i_tesselator_data_t);
}

// --------------------------------------------------------------------------------

static void i_cb_begin_entity(GLenum tipo, struct i_tesselator_data_t *tesselator_data)
{
	assert_no_null(tesselator_data);
	assert(tipo == GL_TRIANGLES);
}

// --------------------------------------------------------------------------------

static void i_cb_end_entity(struct i_tesselator_data_t *tesselator_data)
{
	unsigned long no_points;
	
	assert_no_null(tesselator_data);
	
	no_points = csmarrayc_count_st(tesselator_data->vertexs, i_tesselator_vertex_t);
	assert(no_points % 3 == 0);
}

// --------------------------------------------------------------------------------

static void i_cb_append_vertex_to_tesselator_data(GLvoid *vertex_data, struct i_tesselator_data_t *tesselator_data)
{
	const GLdouble *vertex_data_double;
    struct i_tesselator_vertex_t *tesselator_vertex;
    
	assert_no_null(tesselator_data);

	vertex_data_double = (const GLdouble *)vertex_data;
    
    tesselator_vertex = i_new_tesselator_vertex(
                        vertex_data_double[0], vertex_data_double[1],
                        vertex_data_double[3], vertex_data_double[4], vertex_data_double[5],
                        tesselator_data->current_vertex_on_edge);
    
    csmarrayc_append_element_st(tesselator_data->vertexs, tesselator_vertex, i_tesselator_vertex_t);
}

// --------------------------------------------------------------------------------

static void i_cb_tesselator_error(GLenum numero_error)
{
    fprintf(stdout, "Error teselando: %d\n", numero_error);
}

// --------------------------------------------------------------------------------

static void i_cb_set_edge_flag(GLboolean flag, struct i_tesselator_data_t *tesselator_data)
{
	assert_no_null(tesselator_data);
 
    if (flag)
        tesselator_data->current_vertex_on_edge = CSMTRUE;
    else
        tesselator_data->current_vertex_on_edge = CSMFALSE;
}

// --------------------------------------------------------------------------------

static unsigned long i_total_number_of_points(const csmArrayStruct(i_polygon_t)	*polygons)
{
	unsigned long num_total_points;
	unsigned long i, num_polygons;
	
	num_polygons = csmarrayc_count_st(polygons, i_polygon_t);
	assert(num_polygons > 0);
	
	num_total_points = 0;
	
	for (i = 0; i < num_polygons; i++)
	{
        const struct i_polygon_t *polygon;
        unsigned long num_points_polygon;
        
        polygon = csmarrayc_get_const_st(polygons, i, i_polygon_t);
        assert_no_null(polygon);
        
        num_points_polygon = csmArrPoint2D_count(polygon->points);
		assert(num_points_polygon >= 3);
		
		num_total_points += num_points_polygon;
	}
	
	return num_total_points;
}

// --------------------------------------------------------------------------------

static void i_append_polygons_to_glutesselator(
                        const csmArrayStruct(i_polygon_t) *polygons,
                        GLUtesselator *teselador,
                        GLdouble **points_polygons_gldouble)
{
	unsigned long i, num_polygons;
	unsigned long num_points_csmshape;
	unsigned long idx_point;
	GLdouble *points_polygons_gldouble_loc;
	
	assert_no_null(points_polygons_gldouble);
	
	num_polygons = csmarrayc_count_st(polygons, i_polygon_t);
	assert(num_polygons > 0);
	
    num_points_csmshape = i_total_number_of_points(polygons);
	points_polygons_gldouble_loc = (GLdouble *)CALLOC(6 * num_points_csmshape, GLdouble);
	idx_point = 0;
	
	for (i = 0; i < num_polygons; i++)
	{
        const struct i_polygon_t *polygon;
		unsigned long j, num_points;
        
        polygon = csmarrayc_get_const_st(polygons, i, i_polygon_t);
        assert_no_null(polygon);
        
        num_points = csmArrPoint2D_count(polygon->points);
		assert(num_points >= 3);
		
		gluTessBeginContour(teselador);
		
		for (j = 0; j < num_points; j++)
		{
			double x, y, Nx, Ny, Nz;
			GLdouble *vertex_data;
			
            csmArrPoint2D_get(polygon->points, j, &x, &y);
            csmArrPoint3D_get(polygon->normals, j, &Nx, &Ny, &Nz);
			
			vertex_data = &points_polygons_gldouble_loc[6 * idx_point];
			vertex_data[0] = x;
			vertex_data[1] = y;
			vertex_data[2] = 0;
            vertex_data[3] = Nx;
            vertex_data[4] = Ny;
            vertex_data[5] = Nz;
			
			gluTessVertex(teselador, vertex_data, vertex_data);
			idx_point++;
		}
		
		gluTessEndContour(teselador);
	}
	
	*points_polygons_gldouble = points_polygons_gldouble_loc;
}

// --------------------------------------------------------------------------------

CONSTRUCTOR(static struct i_tesselator_data_t *, i_tesselate_shape_in_triangles, (const csmArrayStruct(i_polygon_t)	*polygons))
{
    struct i_tesselator_data_t *tesselator_data;
    csmArrayStruct(i_tesselator_vertex_t) *tesselator_vertexs;
	GLUtesselator *teselador;
	GLdouble *points_gldouble;
    
    tesselator_vertexs = csmarrayc_new_st_array(0, i_tesselator_vertex_t);
    tesselator_data = i_new_tesselator_data(CSMFALSE, &tesselator_vertexs);
	
	teselador = gluNewTess();
	gluTessCallback(teselador, GLU_TESS_BEGIN_DATA, (void (*)())i_cb_begin_entity);
    gluTessCallback(teselador, GLU_TESS_END_DATA, (void (*)())i_cb_end_entity);
    gluTessCallback(teselador, GLU_TESS_ERROR_DATA, (void (*)())i_cb_tesselator_error);
    gluTessCallback(teselador, GLU_TESS_VERTEX_DATA, (void (*)())i_cb_append_vertex_to_tesselator_data);
    gluTessCallback(teselador, GLU_TESS_EDGE_FLAG_DATA, (void (*)())i_cb_set_edge_flag);

	gluTessBeginPolygon(teselador, tesselator_data);
    i_append_polygons_to_glutesselator(polygons, teselador, &points_gldouble);
	gluTessEndPolygon(teselador);
    
	gluDeleteTess(teselador);
    FREE_PP(&points_gldouble, GLdouble);
    
    return tesselator_data;
}

// --------------------------------------------------------------------------------

static void i_get_vertex_data(
                        const csmArrayStruct(i_tesselator_vertex_t) *vertexs,
                        unsigned long vertex_idx,
						double Xo, double Yo, double Zo,
						double Ux, double Uy, double Uz, double Vx, double Vy, double Vz, double Wx, double Wy, double Wz,
						double *x, double *y, double *z,
						double *Nx, double *Ny, double *Nz,
                        CSMBOOL *is_vertex_on_edge)
{
    const struct i_tesselator_vertex_t *tesselator_vertex;

    assert_no_null(Nx);
    assert_no_null(Ny);
    assert_no_null(Nz);
    assert_no_null(is_vertex_on_edge);
    
    tesselator_vertex = csmarrayc_get_const_st(vertexs, vertex_idx, i_tesselator_vertex_t);
    assert(tesselator_vertex);
    
    csmgeom_coords_2d_to_3d(
						Xo, Yo, Zo,
						Ux, Uy, Uz, Vx, Vy, Vz,
                        tesselator_vertex->x, tesselator_vertex->y,
						x, y, z);
    
    if (tesselator_vertex->Nx == 0. && tesselator_vertex->Ny == 0. && tesselator_vertex->Nz == 0.)
    {
        *Nx = Wx;
        *Ny = Wy;
        *Nz = Wz;
    }
    else
    {
        *Nx = tesselator_vertex->Nx;
        *Ny = tesselator_vertex->Ny;
        *Nz = tesselator_vertex->Nz;
    }
    
    *is_vertex_on_edge = tesselator_vertex->is_vertex_on_edge;
}

// --------------------------------------------------------------------------------

void csmshape2d_draw_3D(
                        const struct csmshape2d_t *shape2d,
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        CSMBOOL draw_edges,
                        struct bsgraphics2_t *graficos)
{
    struct i_tesselator_data_t *tesselator_data;
    unsigned long i, no_vertexs, no_triangles;
    double Wx, Wy, Wz;
    
    assert_no_null(shape2d);
    
    tesselator_data = i_tesselate_shape_in_triangles(shape2d->polygons);
    assert_no_null(tesselator_data);
    
    no_vertexs = csmarrayc_count_st(tesselator_data->vertexs, i_tesselator_vertex_t);
    assert(no_vertexs % 3 == 0);
    
    no_triangles = no_vertexs / 3;
    //assert(no_triangles > 0);
    
    csmmath_cross_product3D(Ux, Uy, Uz, Vx, Vy, Vz, &Wx, &Wy, &Wz);
    
    for (i = 0; i < no_triangles; i++)
    {
        unsigned long base_index;
        double x1, y1, z1, Nx1, Ny1, Nz1;
        double x2, y2, z2, Nx2, Ny2, Nz2;
        double x3, y3, z3, Nx3, Ny3, Nz3;
        CSMBOOL is_vertex_on_edge1, is_vertex_on_edge2, is_vertex_on_edge3;
        
        base_index = i * 3;

        i_get_vertex_data(
                        tesselator_data->vertexs,
                        base_index,
						Xo, Yo, Zo,
						Ux, Uy, Uz, Vx, Vy, Vz, Wx, Wy, Wz,
                        &x1, &y1, &z1, &Nx1, &Ny1, &Nz1, &is_vertex_on_edge1);
        
        i_get_vertex_data(
                        tesselator_data->vertexs,
                        base_index + 1,
						Xo, Yo, Zo,
						Ux, Uy, Uz, Vx, Vy, Vz, Wx, Wy, Wz,
                        &x2, &y2, &z2, &Nx2, &Ny2, &Nz2, &is_vertex_on_edge2);
        
        i_get_vertex_data(
                        tesselator_data->vertexs,
                        base_index + 2,
						Xo, Yo, Zo,
						Ux, Uy, Uz, Vx, Vy, Vz, Wx, Wy, Wz,
                        &x3, &y3, &z3, &Nx3, &Ny3, &Nz3, &is_vertex_on_edge3);
        
        bsgraphics2_escr_triangulo3D_ex(
                        graficos,
                        x1, y1, z1, 0, Nx1, Ny1, Nz1,
                        x2, y2, z2, 0, Nx2, Ny2, Nz2,
                        x3, y3, z3, 0, Nx3, Ny3, Nz3);
    }
    
    i_free_tesselator_data(&tesselator_data);
}
