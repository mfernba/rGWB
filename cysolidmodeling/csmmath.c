//
//  csmmath.c
//  cysolidmodeling
//
//  Created by Manuel Fernández on 22/3/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

#include "csmmath.inl"
#include "csmmath.tli"

#include "cyassert.h"
#include "defmath.tlh"
#include <math.h>

// ------------------------------------------------------------------------------------------

enum csmmath_double_relation_t csmmath_compare_doubles(double value1, double value2, double epsilon)
{
    double diff;
    
    diff = fabs(value1 - value2);
    
    if (diff < epsilon)
        return CSMMATH_EQUAL_VALUES;
    else if (value1 < value1)
        return CSMMATH_VALUE1_LESS_THAN_VALUE2;
    else
        return CSMMATH_VALUE1_GREATER_THAN_VALUE2;
}

// ------------------------------------------------------------------------------------------

CYBOOL csmmath_equal_coords(
                        double x1, double y1, double z1,
                        double x2, double y2, double z2,
                        double epsilon)
{
    double diff;
    
    diff = CUAD(x1 - x2) + CUAD(y1 - y2) + CUAD(z1 - z2);
    
    if (diff < CUAD(epsilon))
        return CIERTO;
    else
        return FALSO;
}

// ------------------------------------------------------------------------------------------

double csmmath_length_vector3D(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
}

// ------------------------------------------------------------------------------------------

double csmmath_distance_3D(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return csmmath_length_vector3D(x2 - x1, y2 - y1, z2 - z1);
}

// ------------------------------------------------------------------------------------------

void csmmath_make_unit_vector3D(double *Ux, double *Uy, double *Uz)
{
    double length;
    
    assert_no_null(Ux);
    assert_no_null(Uy);
    assert_no_null(Uz);
    
    length = csmmath_length_vector3D(*Ux, *Uy, *Uz);
    assert(length > 0.);
    
    *Ux /= length;
    *Uy /= length;
    *Uz /= length;
}

//-------------------------------------------------------------------------------------------

void csmmath_vector_between_two_3D_points(double x1, double y1, double z1, double x2, double y2, double z2, double *Ux, double *Uy, double *Uz)
{
	assert_no_null(Ux);
	assert_no_null(Uy);
	assert_no_null(Uz);
    
	*Ux = x2 - x1;
	*Uy = y2 - y1;
	*Uz = z2 - z1;
}

// ------------------------------------------------------------------------------------------

void csmmath_unit_vector_between_two_3D_points(double x1, double y1, double z1, double x2, double y2, double z2, double *Ux, double *Uy, double *Uz)
{
    csmmath_vector_between_two_3D_points(x1, y1, z1, x2, y2, z2, Ux, Uy, Uz);
    csmmath_make_unit_vector3D(Ux, Uy, Uz);
}

// ------------------------------------------------------------------------------------------

double csmmath_dot_product3D(double Ux, double Uy, double Uz, double Vx, double Vy, double Vz)
{
    return Ux * Vx + Uy * Vy + Uz * Vz;
}

//-------------------------------------------------------------------------------------------

void csmmath_cross_product3D(double Ux, double Uy, double Uz, double Vx, double Vy, double Vz, double *Wx, double *Wy, double *Wz)
{
	assert_no_null(Wx);
	assert_no_null(Wy);
	assert_no_null(Wz);

	*Wx = (Uy * Vz - Vy * Uz);
	*Wy = -(Ux * Vz - Vx * Uz);
	*Wz = (Ux * Vy - Vx * Uy);
}

//-------------------------------------------------------------------------------------------

double csmmath_distance_from_point_to_line3D(
						double x, double y, double z,
						double Xo_recta, double Yo_recta, double Zo_recta, double Ux_recta, double Uy_recta, double Uz_recta)
{
	double Ux_recta_punto, Uy_recta_punto, Uz_recta_punto;
    double Wx, Wy, Wz;
    
    csmmath_vector_between_two_3D_points(Xo_recta, Yo_recta, Zo_recta, x, y, z, &Ux_recta_punto, &Uy_recta_punto, &Uz_recta_punto);
    csmmath_cross_product3D(Ux_recta_punto, Uy_recta_punto, Uz_recta_punto, Ux_recta, Uy_recta, Uz_recta, &Wx, &Wy, &Wz);
    
    return csmmath_length_vector3D(Wx, Wy, Wz);
}

//-------------------------------------------------------------------------------------------

static void i_valida_es_vector_unitario_3D(double Ux, double Uy, double Uz)
{
	double modulo;

	modulo = csmmath_length_vector3D(Ux, Uy, Uz);
	assert(modulo - 1. < 1.e-6);
}

//-------------------------------------------------------------------------------------------

static void i_point_on_line3D(
							double Xo_recta, double Yo_recta, double Zo_recta, double Ux_recta, double Uy_recta, double Uz_recta,
							double posicion,
							double *x, double *y, double *z)
{
	assert_no_null(x);
	assert_no_null(y);
	assert_no_null(z);

	*x = Xo_recta + posicion * Ux_recta;
	*y = Yo_recta + posicion * Uy_recta;
	*z = Zo_recta + posicion * Uz_recta;
}

//-------------------------------------------------------------------------------------------

static CYBOOL i_son_rectas_paralelas_segun_determinante(double determinante)
{
	if (fabs(determinante) < 1.e-6)
		return CIERTO;
	else
		return FALSO;
}

//-------------------------------------------------------------------------------------------

void csmmath_nearer_points_between_two_lines3D(
							double Xo_recta1, double Yo_recta1, double Zo_recta1, double Ux_recta1, double Uy_recta1, double Uz_recta1,
							double Xo_recta2, double Yo_recta2, double Zo_recta2, double Ux_recta2, double Uy_recta2, double Uz_recta2,
							double *x_mas_cercano_recta1, double *y_mas_cercano_recta1, double *z_mas_cercano_recta1,
							double *x_mas_cercano_recta2, double *y_mas_cercano_recta2, double *z_mas_cercano_recta2)
{
	double determinante;
	double prod_escalar_vectores_directores;

	i_valida_es_vector_unitario_3D(Ux_recta1, Uy_recta1, Uz_recta1);
	i_valida_es_vector_unitario_3D(Ux_recta2, Uy_recta2, Uz_recta2);
	assert_no_null(x_mas_cercano_recta1);
	assert_no_null(y_mas_cercano_recta1);
	assert_no_null(z_mas_cercano_recta1);
	assert_no_null(x_mas_cercano_recta2);
	assert_no_null(y_mas_cercano_recta2);
	assert_no_null(z_mas_cercano_recta2);

	prod_escalar_vectores_directores = csmmath_dot_product3D(Ux_recta1, Uy_recta1, Uz_recta1, Ux_recta2, Uy_recta2, Uz_recta2);
	determinante = CUAD(prod_escalar_vectores_directores) - 1.;

	if (i_son_rectas_paralelas_segun_determinante(determinante) == CIERTO)
	{
		*x_mas_cercano_recta1 = Xo_recta1;
		*y_mas_cercano_recta1 = Yo_recta1;
		*z_mas_cercano_recta1 = Zo_recta1;

		*x_mas_cercano_recta2 = Xo_recta2;
		*y_mas_cercano_recta2 = Yo_recta2;
		*z_mas_cercano_recta2 = Zo_recta2;
	}
	else
	{
		double Ux_entre_rectas, Uy_entre_rectas, Uz_entre_rectas;
		double inv_determinante;
		double prod_escalar1, prod_escalar2;
		double t1, t2;

		Ux_entre_rectas =  Xo_recta2 - Xo_recta1;
		Uy_entre_rectas =  Yo_recta2 - Yo_recta1;
		Uz_entre_rectas =  Zo_recta2 - Zo_recta1;

		inv_determinante = 1. / determinante;
		prod_escalar1 = csmmath_dot_product3D(Ux_recta1, Uy_recta1, Uz_recta1, Ux_entre_rectas, Uy_entre_rectas, Uz_entre_rectas);
		prod_escalar2 = csmmath_dot_product3D(Ux_recta2, Uy_recta2, Uz_recta2, Ux_entre_rectas, Uy_entre_rectas, Uz_entre_rectas);

		t1 = inv_determinante * (-prod_escalar1 + prod_escalar_vectores_directores * prod_escalar2);
		t2 = inv_determinante * (-prod_escalar_vectores_directores * prod_escalar1 + prod_escalar2);

		i_point_on_line3D(Xo_recta1, Yo_recta1, Zo_recta1, Ux_recta1, Uy_recta1, Uz_recta1, t1, x_mas_cercano_recta1, y_mas_cercano_recta1, z_mas_cercano_recta1);
		i_point_on_line3D(Xo_recta2, Yo_recta2, Zo_recta2, Ux_recta2, Uy_recta2, Uz_recta2, t2, x_mas_cercano_recta2, y_mas_cercano_recta2, z_mas_cercano_recta2);
	}
}

//-------------------------------------------------------------------------------------------

CYBOOL csmmath_is_point_in_segment3D(
						double x, double y, double z,
						double x1, double y1, double z1, double x2, double y2, double z2,
						double precision, 
						double *posicion_relativa_opc)
{
    CYBOOL pertenece;
    double posicion_relativa_loc;
    double distancia_con_signo_a_recta;
    double Ux_seg, Uy_seg, Uz_seg;

	assert(precision > 0.);
	
	Ux_seg = x2 - x1;
	Uy_seg = y2 - y1;
	Uz_seg = z2 - z1;
    csmmath_make_unit_vector3D(&Ux_seg, &Uy_seg, &Uz_seg);
    
    distancia_con_signo_a_recta = csmmath_distance_from_point_to_line3D(
                        x, y, z,
                        x1, y1, z1, Ux_seg, Uy_seg, Uz_seg);
    
    if (fabs(distancia_con_signo_a_recta) < precision)
    {
        double longitud_segmento, distancia_punto_a_origen_segmento;
        
        distancia_punto_a_origen_segmento = csmmath_length_vector3D(x - x1, y - y1, z - z1);
        longitud_segmento = csmmath_distance_3D(x1, y1, z1, x2, y2, z2);
        
        posicion_relativa_loc  = distancia_punto_a_origen_segmento / longitud_segmento;
        
        if (posicion_relativa_loc + precision > 0. && posicion_relativa_loc - precision < 1.)
        {
            pertenece = CIERTO;
        }
        else
        {
            pertenece = FALSO;
            posicion_relativa_loc = 0.;
        }
    }
    else
    {
        pertenece = FALSO;
        posicion_relativa_loc = 0.;
    }
    
    ASIGNA_OPC(posicion_relativa_opc, posicion_relativa_loc);
    
    return pertenece;
}

//-------------------------------------------------------------------------------------------

CYBOOL csmmath_exists_intersection_between_two_lines3D(
						double Xo_recta1, double Yo_recta1, double Zo_recta1, double Ux_recta1, double Uy_recta1, double Uz_recta1,
						double Xo_recta2, double Yo_recta2, double Zo_recta2, double Ux_recta2, double Uy_recta2, double Uz_recta2,
						double precision, 
						double *x_corte, double *y_corte, double *z_corte)
{
	double x_mas_cercano_recta1, y_mas_cercano_recta1, z_mas_cercano_recta1;
	double x_mas_cercano_recta2, y_mas_cercano_recta2, z_mas_cercano_recta2;
	double distancia_entre_puntos;
	
	assert(precision > 0.);
	assert_no_null(x_corte);
	assert_no_null(y_corte);
	assert_no_null(z_corte);

	csmmath_nearer_points_between_two_lines3D(
							Xo_recta1, Yo_recta1, Zo_recta1, Ux_recta1, Uy_recta1, Uz_recta1,
							Xo_recta2, Yo_recta2, Zo_recta2, Ux_recta2, Uy_recta2, Uz_recta2,
							&x_mas_cercano_recta1, &y_mas_cercano_recta1, &z_mas_cercano_recta1,
							&x_mas_cercano_recta2, &y_mas_cercano_recta2, &z_mas_cercano_recta2);
							
	distancia_entre_puntos	= csmmath_distance_3D(
							x_mas_cercano_recta1, y_mas_cercano_recta1, z_mas_cercano_recta1,
							x_mas_cercano_recta2, y_mas_cercano_recta2, z_mas_cercano_recta2);
							
	if (distancia_entre_puntos < precision)
	{	
		*x_corte = x_mas_cercano_recta1;
		*y_corte = y_mas_cercano_recta1;
		*z_corte = z_mas_cercano_recta1;
	
		return CIERTO;
	}
	else
	{
		return FALSO;
	}
}

//-------------------------------------------------------------------------------------------

CYBOOL csmmath_exists_intersection_between_two_segments3D(
						double x1_seg1, double y1_seg1, double z1_seg1, double x2_seg1, double y2_seg1, double z2_seg1, 
						double x1_seg2, double y1_seg2, double z1_seg2, double x2_seg2, double y2_seg2, double z2_seg2, 
						double precision, 
						double *x_corte_opc, double *y_corte_opc, double *z_corte_opc,
						double *posicion_relativa1_opc, 
						double *posicion_relativa2_opc)
{
	CYBOOL hay_interseccion;
	double Ux1, Uy1, Uz1, Ux2, Uy2, Uz2;
	double x_corte_loc, y_corte_loc, z_corte_loc;
	double posicion_relativa1_loc, posicion_relativa2_loc;
	
	csmmath_unit_vector_between_two_3D_points(x1_seg1, y1_seg1, z1_seg1, x2_seg1, y2_seg1, z2_seg1, &Ux1, &Uy1, &Uz1);
	csmmath_unit_vector_between_two_3D_points(x1_seg2, y1_seg2, z1_seg2, x2_seg2, y2_seg2, z2_seg2, &Ux2, &Uy2, &Uz2);
	
	if (csmmath_exists_intersection_between_two_lines3D(
						x1_seg1, y1_seg1, z1_seg1, Ux1, Uy1, Uz1, 
						x1_seg2, y1_seg2, z1_seg2, Ux2, Uy2, Uz2,
						precision, 
						&x_corte_loc, &y_corte_loc, &z_corte_loc) == FALSO)
	{	
		hay_interseccion = FALSO;
		
		x_corte_loc = 0.;
		y_corte_loc = 0.;
		z_corte_loc = 0.;
		
		posicion_relativa1_loc = 0.;
		posicion_relativa2_loc = 0.;
	}
	else
	{
		if (csmmath_is_point_in_segment3D(
						x_corte_loc, y_corte_loc, z_corte_loc,
						x1_seg1, y1_seg1, z1_seg1, x2_seg1, y2_seg1, z2_seg1, 
						precision, 
						&posicion_relativa1_loc) == FALSO)
		{	
			hay_interseccion = FALSO;

			x_corte_loc = 0.;
			y_corte_loc = 0.;
			z_corte_loc = 0.;

			posicion_relativa1_loc = 0.;
			posicion_relativa2_loc = 0.;
		}
		else if (csmmath_is_point_in_segment3D(
						x_corte_loc, y_corte_loc, z_corte_loc,
						x1_seg2, y1_seg2, z1_seg2, x2_seg2, y2_seg2, z2_seg2, 
						precision, 
						&posicion_relativa2_loc) == FALSO)
		{
			hay_interseccion = FALSO;

			x_corte_loc = 0.;
			y_corte_loc = 0.;
			z_corte_loc = 0.;

			posicion_relativa1_loc = 0.;
			posicion_relativa2_loc = 0.;
		}
		else
		{	
			hay_interseccion = FALSO;
		}
	}
	
	ASIGNA_OPC(x_corte_opc, x_corte_loc);
	ASIGNA_OPC(y_corte_opc, y_corte_loc);
	ASIGNA_OPC(z_corte_opc, z_corte_loc);
	ASIGNA_OPC(posicion_relativa1_opc, posicion_relativa1_loc);
	ASIGNA_OPC(posicion_relativa2_opc, posicion_relativa2_loc);

	return hay_interseccion;
}

// ------------------------------------------------------------------------------------------

double csmmath_signed_distance_point_to_plane(double x, double y, double z, double A, double B, double C, double D)
{
	return A * x + B * y + C * z + D;
}
