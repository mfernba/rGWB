// Geometry Utiliy Functions...

#include "csmfwddecl.hxx"

void csmgeom_coords_2d_to_3d(
						double Xo, double Yo, double Zo, 
						double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
						double x_2d, double y_2d,
						double *x_3d, double *y_3d, double *z_3d);

void csmgeom_project_coords_3d_to_2d(
                        double Xo, double Yo, double Zo,
                        double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
                        double x_3d, double y_3d, double z_3d,
                        double *x_2d, double *y_2d);

void csmgeom_coords_3d_local_to_global(
                        double Xo, double Yo, double Zo,
						double Ux, double Uy, double Uz, double Vx, double Vy, double Vz,
						double x_3d_local, double y_3d_local, double z_3d_local,
                        double *x_3d, double *y_3d, double *z_3d);
