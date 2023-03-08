#ifndef _defs_h
#define _defs_h

#include <math.h>

#define  deg2rad(d)		((d)*M_PI/180.0)
#define  rad2deg(d)		((d)*180.0/M_PI)
#define  min(a,b)		((a)<(b)? (a):(b))
#define  max(a,b)		((a)>(b)? (a):(b))
#define  SQ(x)			((x)*(x))

#define  EPS			(1e-06)
#define  is_zero(d)		(fabs(d) < EPS)

#define  VPDIST		1.0		/* distance between camera and viewport */
#define  ROT_UNIT	deg2rad(4.)
#define  MOV_UNIT	0.2

#define  BGCOLOR	color_new(0,0,0)

#define  MAX_RECUR	2	/* max recursion depth for reflected rays */

enum {
	KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
	KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_DEL, KEY_PGDN, KEY_SPC,
	KEY_W, KEY_A, KEY_S, KEY_D, KEY_Q, KEY_E, KEY_ESC, KEY_OTHER
};

#endif
