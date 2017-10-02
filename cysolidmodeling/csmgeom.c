// Geometry Utiliy Functions...

#include "csmgeom.inl"

#include "csmmath.inl"

#include "csmassert.inl"

//-------------------------------------------------------------------------------------------

void csmgeom_coords_2d_to_3d(
                        double Xo, double Yo, double Zo,
						double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
						double x_2d, double y_2d,
						double *x_3d, double *y_3d, double *z_3d)
{
	assert_no_null(x_3d);
	assert_no_null(y_3d);
	assert_no_null(z_3d);
	
	*x_3d = Xo + x_2d * Ux + y_2d * Vx;
	*y_3d = Yo + x_2d * Uy + y_2d * Vy;
	*z_3d = Zo + x_2d * Uz + y_2d * Vz;	
}

//-------------------------------------------------------------------------------------------

void csmgeom_project_coords_3d_to_2d(
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        double x_3d, double y_3d, double z_3d,
                        double *x_2d, double *y_2d)
{
	double x_2d_loc, y_2d_loc, z_2d_loc;
	double x_2d_plano, y_2d_plano;
	double Wx, Wy, Wz;
	
	assert_no_null(x_2d);
	assert_no_null(y_2d);
	
	csmmath_cross_product3D(Ux, Uy, Uz, Vx, Vy, Vz, &Wx, &Wy, &Wz);
	
	x_2d_loc = x_3d - Xo;
	y_2d_loc = y_3d - Yo;
	z_2d_loc = z_3d - Zo;
	
	x_2d_plano = x_2d_loc * Ux + y_2d_loc * Uy + z_2d_loc * Uz;
	y_2d_plano = x_2d_loc * Vx + y_2d_loc * Vy + z_2d_loc * Vz;
	
	*x_2d = x_2d_plano;
	*y_2d = y_2d_plano;}
