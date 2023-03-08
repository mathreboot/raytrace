#ifndef _vec_h
#define _vec_h

#include "types.h"

vec_t vec_new( double x, double y, double z );

double vec_len( vec_t );
vec_t vec_normalize( vec_t );

vec_t vec_scale( double, vec_t );
vec_t vec_inv( vec_t );

vec_t vec_add( vec_t, vec_t );
vec_t vec_sub( vec_t, vec_t );

double vec_dot( vec_t, vec_t );
vec_t vec_cross( vec_t, vec_t );

double vec_angle_cos( vec_t, vec_t );
vec_t vec_rotated( vec_t v, vec_t axis, double th );
vec_t vec_reflected( vec_t v, vec_t axis );

void vec_print( const vec_t*, const char* name );

#endif
