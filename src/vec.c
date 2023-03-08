#include <stdio.h>
#include <math.h>
#include "defs.h"
#include "vec.h"

vec_t
vec_new( double x, double y, double z )
{
	vec_t v;
	v.x = x; v.y = y; v.z = z;
	return v;
}

double
vec_len( vec_t v )
{
	return sqrt(SQ(v.x) + SQ(v.y) + SQ(v.z));
}

vec_t
vec_normalize( vec_t v )
{
	return vec_scale((1./vec_len(v)), v);
}

vec_t
vec_scale( double k, vec_t v )
{
	return vec_new((k * v.x), (k * v.y), (k * v.z));
}

vec_t
vec_inv( vec_t v )
{
	return vec_new(-v.x, -v.y, -v.z);
}

vec_t
vec_add( vec_t a, vec_t b )
{
	return vec_new((a.x + b.x), (a.y + b.y), (a.z + b.z));
}

vec_t
vec_sub( vec_t a, vec_t b )
{
	return vec_new((a.x - b.x), (a.y - b.y), (a.z - b.z));
}

double
vec_dot( vec_t a, vec_t b )
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

vec_t
vec_cross( vec_t a, vec_t b )
{
	return vec_new( ((a.y * b.z) - (a.z * b.y)),
					(-(a.x * b.z) + (a.z * b.x)),
					((a.x * b.y) - (a.y * b.x)) );
}

double
vec_angle_cos( vec_t a, vec_t b )
{
	return vec_dot(a, b)/(vec_len(a) * vec_len(b));
}

/* v and axis are perpendicular */
/* rotated = cos(th)*v + sin(th)*(axis x v) */

vec_t
vec_rotated( vec_t v, vec_t axis, double th )
{
	return vec_add( vec_scale(cos(th), v),
					vec_scale(sin(th), vec_cross(axis, v)) );
}

/* reflected ray */

vec_t
vec_reflected( vec_t v, vec_t axis )
{
	double len_sq = SQ(axis.x) + SQ(axis.y) + SQ(axis.z);
	return vec_sub(vec_scale(2.*vec_dot(v, axis)/len_sq, axis), v);
}

void
vec_print( const vec_t* v, const char* name )
{
	printf("%s=(%lg, %lg, %lg) ", name, v->x, v->y, v->z);
}

/* EOF */
