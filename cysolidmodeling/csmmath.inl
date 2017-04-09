// Basic Mathematic Functions...

#include "csmfwddecl.hxx"

CYBOOL csmmath_equal_coords(
                    double x1, double y1, double z1,
                    double x2, double y2, double z2,
                    double epsilon);


// Vectors...

double csmmath_length_vector3D(double x, double y, double z);

void csmmath_make_unit_vector3D(double *Ux, double *Uy, double *Uz);

double csmmath_dot_product(double Ux, double Uy, double Uz, double Vx, double Vy, double Vz);


// Planes...

double csmmath_signed_distance_point_to_plane(double x, double y, double z, double A, double B, double C, double D);
