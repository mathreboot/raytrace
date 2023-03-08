#ifndef _miscutil_h
#define _miscutil_h

#include "types.h"

color_t color_new( int r, int g, int b );
color_t color_fade( color_t, double lum );
color_t color_fade3( color_t, vec_t lum );

double l2dist( double x0, double y0, double x1, double y1 );

int get_numcores();

#endif
