#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "conf.h"
#include "render.h"
#include "miscutil.h"
#include "gr_sdl.h"
#include "camera.h"

static bool process_key( int key, camera_t* );

int
main( int ac, char** av )
{
	if( ac != 4 ) {
		fprintf(stderr,"usage: %s conf_file num_threads (0 for 'max') cam_no (0-based)\n", av[0]);
		exit(1);
	}

	int nth = atoi(av[2]);
	int cam = atoi(av[3]);

	conf_t conf;
	if( cam < 0 || conf_load(&conf, av[1]) == false || cam >= conf.ncams ) {
		conf_clear(&conf);
		exit(1);
	}
	int ncores = get_numcores();
	if( nth == 0 ) nth = ncores;
	else if( nth < 0 || nth > ncores ) {
		fprintf(stderr,"invalid number of threads (max %d)\n", ncores);
		exit(1);
	}
	printf("using %d / %d cpu cores\n", nth, ncores);
#ifdef DEBUG
	conf_print(&conf);
#endif

	void* gr = gr_init(conf.res_x, conf.res_y, "mini ray-tracer");

	if( render(gr, &conf, cam, nth) == false ) goto fin;

	/* main loop */
	for(;;) {
		int key = gr_readkey(gr);
		if( key < 0 || key == KEY_ESC ) { /* quit */
			break;
		}
		if( key >= KEY_0 && key <= KEY_9 ) { /* change cam */
			int newcam = key - KEY_0;
			if( newcam < conf.ncams && newcam != cam ) {
				cam = newcam;
				if( render(gr, &conf, cam, nth) == false ) break;
			}
		}
		else if( process_key(key, &(conf.cams[cam])) ) {
			if( render(gr, &conf, cam, nth) == false ) break;
		}
	}

  fin:;
	gr_finish(gr);
	conf_clear(&conf);
	return 0;
}

static bool
process_key( int key, camera_t* pcam )
{
	switch( key ) {
		case KEY_A:		cam_move_x(pcam, -MOV_UNIT); break;
		case KEY_D:		cam_move_x(pcam, +MOV_UNIT); break;
		case KEY_Q:		cam_move_y(pcam, -MOV_UNIT); break;
		case KEY_E:		cam_move_y(pcam, +MOV_UNIT); break;
		case KEY_W:		cam_move_z(pcam, +MOV_UNIT); break;
		case KEY_S:		cam_move_z(pcam, -MOV_UNIT); break;

		case KEY_UP:	cam_rotate_x(pcam, +ROT_UNIT); break;
		case KEY_DOWN:	cam_rotate_x(pcam, -ROT_UNIT); break;
		case KEY_LEFT:	cam_rotate_y(pcam, +ROT_UNIT); break;
		case KEY_RIGHT:	cam_rotate_y(pcam, -ROT_UNIT); break;
		case KEY_DEL:	cam_rotate_z(pcam, -ROT_UNIT); break;
		case KEY_PGDN:	cam_rotate_z(pcam, +ROT_UNIT); break;

		case KEY_SPC:	cam_print(pcam); break;
		default: return false;
	}
	return true;
}

/* EOF */
