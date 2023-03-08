#ifndef _camera_h
#define _camera_h

#include "types.h"

void cam_move_x( camera_t*, double );
void cam_move_y( camera_t*, double );
void cam_move_z( camera_t*, double );

void cam_rotate_x( camera_t*, double );
void cam_rotate_y( camera_t*, double );
void cam_rotate_z( camera_t*, double );

void cam_print( camera_t* );

#endif
