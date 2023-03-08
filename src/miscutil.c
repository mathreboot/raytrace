#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "defs.h"
#include "types.h"
#include "miscutil.h"

color_t
color_new( int r, int g, int b )
{
	color_t c;
	c.r = r; c.g = g; c.b = b;
	return c;
}

color_t
color_fade( color_t c, double lum )
{
	if( lum < 0 ) lum = 0;
	else if( lum > 1 ) lum = 1;
	return color_new((int)(c.r * lum), (int)(c.g * lum), (int)(c.b * lum));
}

color_t
color_fade3( color_t c, vec_t lum )
{
	if( lum.x < 0 || lum.x > 1 || lum.y < 0 || lum.y > 1 || lum.z < 0 || lum.z > 1 ) {
		fprintf(stderr,"color_fade3: invalid lum vector (%lf, %lf, %lf)\n", lum.x, lum.y, lum.z);
		return color_new(0,0,0);
	}
	return color_new((int)(c.r * lum.x), (int)(c.g * lum.y), (int)(c.b * lum.z));
}

double
l2dist( double x0, double y0, double x1, double y1 )
{
	double dx = x0 - x1;
	double dy = y0 - y1;
	return sqrt(SQ(dx) + SQ(dy));
}

int
get_numcores()
{
	return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

/* EOF */
