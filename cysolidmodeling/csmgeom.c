// Geometry Utiliy Functions...

#include "csmgeom.inl"

#include "cyassert.h"

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
