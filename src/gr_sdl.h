#ifndef _gr_sdl_h
#define _gr_sdl_h

#include "types.h"

void* gr_init( int w, int h, const char* title );
void gr_finish( void* );

void gr_flush( void* );
int gr_clear( void*, color_t );
int gr_save_bmp( void*, char* fname );

int gr_putpixel( void*, int x, int y, color_t );

int gr_readkey( void* not_used );

#endif
