#include <stdio.h>
#include "vec.h"
#include "camera.h"

void
cam_move_x( camera_t* pcam, double d )
{
	vec_t side = vec_cross(pcam->ov, pcam->up);
	pcam->vp = vec_add(pcam->vp, vec_scale(d, side));
}

void
cam_move_y( camera_t* pcam, double d )
{
	pcam->vp = vec_add(pcam->vp, vec_scale(d, pcam->up));
}

void
cam_move_z( camera_t* pcam, double d )
{
	pcam->vp = vec_add(pcam->vp, vec_scale(d, pcam->ov));
}

void
cam_rotate_x( camera_t* pcam, double th )
{
	vec_t side = vec_cross(pcam->ov, pcam->up);
	pcam->up = vec_rotated(pcam->up, side, th);
	pcam->ov = vec_rotated(pcam->ov, side, th);
}

void
cam_rotate_y( camera_t* pcam, double th )
{
	pcam->ov = vec_rotated(pcam->ov, pcam->up, th);
}

void
cam_rotate_z( camera_t* pcam, double th )
{
	pcam->up = vec_rotated(pcam->up, pcam->ov, th);
}

void
cam_print( camera_t* pcam )
{
	printf("current camera: ");
	vec_print(&pcam->vp, "vp");
	vec_print(&pcam->ov, "ov");
	vec_print(&pcam->up, "up");
	putchar('\n');
}

/* EOF */
