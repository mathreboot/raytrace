#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "defs.h"
#include "conf.h"
#include "gr_sdl.h"
#include "vec.h"
#include "miscutil.h"
#include "gr_sdl.h"
#include "render.h"

/*--------------------------------------------------*/
/* determine viewport vectors (scr00, stepx, stepy) */

static void
get_vp_vectors( const camera_t* pcam, int sx, int sy, vec_t* pscr00, vec_t* pstepx, vec_t* pstepy )
{
	double w = VPDIST * tan(deg2rad(pcam->fov)/2.);
	double h = w * (sy -1.)/(sx -1.);
	double t = (2.* w)/(sx -1.);
	vec_t side = vec_cross(pcam->ov, pcam->up);

	*pstepx = vec_scale(t, side);
	*pstepy = vec_scale(-t, pcam->up);
	*pscr00 = vec_sub( vec_add(vec_scale(VPDIST, pcam->ov), vec_scale(h, pcam->up)),
						vec_scale(w, side) );
}

/*-------------------------------------------------*/
/* get 't' parameter of nearest intersection point */

static double
get_intersect_sp( vec_t eye, vec_t ray, const obj_t* obj )
{
	vec_t v = vec_sub(eye, obj->u.sp.center);
	double a = vec_dot(ray, ray);
	double b = 2.* vec_dot(ray, v);
	double c = vec_dot(v, v) - SQ(obj->u.sp.r);

	double D = b*b - 4*a*c;
	return (D < 0) ? INFINITY : min((-b+sqrt(D))/(2*a), (-b-sqrt(D))/(2*a));
}

static double
get_intersect_pl( vec_t eye, vec_t ray, const obj_t* obj )
{
	double den = vec_dot(obj->ov, ray);
	if( is_zero(den) ) return INFINITY; /* parallel */
	return (vec_dot(obj->ov, vec_sub(obj->u.pl.coord, eye)) / den);
}

static bool
right_side( vec_t v, vec_t p, vec_t n )
{
	return !(vec_dot(vec_cross(v, p), n) > 0);
}

static double
get_intersect_tr( vec_t eye, vec_t ray, const obj_t* obj )
{
	/* test if ray intersects with plane */
	double den = vec_dot(obj->ov, ray);
	if( is_zero(den) ) return INFINITY; /* parallel */

	/* test if the intersection point is inside triangle */
	double t = vec_dot(obj->ov, vec_sub(obj->u.tr.p1, eye)) / den;
	vec_t v_op = vec_add(eye, vec_scale(t, ray));  /* intersection point */

	return (right_side(obj->u.tr.v12, vec_sub(v_op, obj->u.tr.p1), obj->ov) &&
			right_side(obj->u.tr.v23, vec_sub(v_op, obj->u.tr.p2), obj->ov) &&
			right_side(obj->u.tr.v31, vec_sub(v_op, obj->u.tr.p3), obj->ov)) ? t : INFINITY;
}

static double
get_intersect_sq( vec_t eye, vec_t ray, const obj_t* obj )
{
	double den = vec_dot(obj->ov, ray);
	if( is_zero(den) ) return INFINITY; /* parallel */

	double t = vec_dot(obj->ov, vec_sub(obj->u.sq.center, eye)) / den;
	vec_t v_op = vec_add(eye, vec_scale(t, ray));  /* intersection point */

	return (right_side(obj->u.sq.v12, vec_sub(v_op, obj->u.sq.p1), obj->ov) &&
			right_side(obj->u.sq.v23, vec_sub(v_op, obj->u.sq.p2), obj->ov) &&
			right_side(obj->u.sq.v34, vec_sub(v_op, obj->u.sq.p3), obj->ov) &&
			right_side(obj->u.sq.v41, vec_sub(v_op, obj->u.sq.p4), obj->ov)) ? t : INFINITY;
}

/*--------------------------------------------------------------*/
/* determine closest object; return object index and 't' param  */

typedef struct {
	objtype_t type;
	double (*func)(vec_t, vec_t, const obj_t*);
} funcmap_t;

static funcmap_t fmap[] = {
	{ O_SP, get_intersect_sp },
	{ O_PL, get_intersect_pl },
	{ O_TR, get_intersect_tr },
	{ O_SQ, get_intersect_sq },
};

static double
get_intersect_obj( vec_t eye, vec_t ray, const obj_t* obj )
{
	double t = INFINITY;
	for( int i=0; i<(sizeof(fmap)/sizeof(funcmap_t)); i++ ) {
		if( obj->type == fmap[i].type )
			t = (fmap[i].func)(eye, ray, obj);
	}
	return t;
}

static obj_t*
closest_obj( const obj_t* obj_arr, int nobjs, vec_t eye, vec_t ray, double tmin, double tmax, double* pt )
{
	double cl_t = tmax;
	obj_t* cl_obj = NULL;

	for( int i=0; i<nobjs; i++ ) {
		const obj_t* obj = &(obj_arr[i]);
		double t = get_intersect_obj(eye, ray, obj);
		if( tmin < t && t < cl_t ) {
			cl_t = t;
			cl_obj = (obj_t*) obj;
		}
	}
	if( pt ) *pt = cl_t;
	return cl_obj;
}

/*-------------------------------------*/
/* trace a ray & determine final color */

static vec_t
obj_normal( const obj_t* obj, vec_t v_op )
{
	if( obj->type == O_SP )
		return vec_normalize(vec_sub(v_op, obj->u.sp.center));
	/* TODO: O_CY */
	else
		return obj->ov;
}

static double
phong_refl( vec_t v_nm, vec_t v_pl, vec_t v_pe, double kd, double ks, double shine )
{
	double m = 0;
	double cos_a = vec_angle_cos(v_nm, v_pl);
	if( cos_a > 0 ) {
		m = (kd * cos_a);  /* diffuse */
		if( !(shine < 1) ) {
			double cos_b = vec_angle_cos(vec_reflected(v_pl, v_nm), v_pe);
			if( cos_b > 0 )
				m += (ks * pow(cos_b, shine));  /* specular */
		}
	}
	return m;
}

static vec_t
compute_lighting( const conf_t* pconf, vec_t v_op, vec_t v_pe, vec_t v_nm, double shine )
{
	vec_t lum = vec_new(0,0,0); /* just a triple of [0,1]; RGB multiplier */

	for( int i=0; i < pconf->nlights; i++ ) {
		light_t* l = &(pconf->lights[i]);
		vec_t lcol3 = vec_scale(1/255., vec_new(l->col.r, l->col.g, l->col.b));

		if( l->type == L_AMB ) {
			lum = vec_add(lum, vec_scale(l->lum, lcol3));
		}
		else {	/* L_PT */
			vec_t v_pl = vec_sub(l->coord, v_op);
			if( closest_obj(pconf->objs, pconf->nobjs, v_op, v_pl, EPS, 1., NULL) != NULL )
				continue;	/* we're in the shadow */

			double m = phong_refl(v_nm, v_pl, v_pe, pconf->kd, pconf->ks, shine);
			if( m > 0 )
				lum = vec_add(lum, vec_scale((l->lum)*m, lcol3));
		}
	}
	return lum;
}

static color_t
trace_ray1( const conf_t* pconf, vec_t eye, vec_t ray, double tmin, double tmax, int lim )
{
	/* determine closest object */
	double cl_t;
	obj_t* cl_obj = closest_obj(pconf->objs, pconf->nobjs, eye, ray, tmin, tmax, &cl_t);
	if( cl_obj == NULL )
		return BGCOLOR; /* no intersecting object */

	vec_t v_pe = vec_scale(-cl_t, ray);		/* P -> eye */
	vec_t v_op = vec_sub(eye, v_pe);		/* O -> P */
	vec_t v_nm = obj_normal(cl_obj, v_op);	/* object normal */
	if( is_planar(cl_obj->type) && vec_dot(v_nm, v_pe) < 0 ) /* opposite to the eye? */
		v_nm = vec_inv(v_nm);

	/* determine final luminosity multiplier */
	vec_t lum = compute_lighting(pconf, v_op, v_pe, v_nm, cl_obj->shine);
	color_t col_local = color_fade3(cl_obj->col, lum);

	if( lim <= 0 || is_zero(cl_obj->refl) ) {
		return col_local;
	}
	/* get reflected color via recursion */
	vec_t v_refl = vec_reflected(vec_scale(-1, ray), v_nm);
	color_t col_refl = trace_ray1(pconf, v_op, v_refl, EPS, INFINITY, lim-1);

	/* return weighted avg. */
	color_t c1 = color_fade(col_local, (1.- cl_obj->refl));
	color_t c2 = color_fade(col_refl, cl_obj->refl);
	return color_new((c1.r + c2.r), (c1.g + c2.g), (c1.b + c2.b));
}

/*-----------------------*/
/* multi-threaded worker */

typedef struct {
	int tid;
	int nth;
	const conf_t* pconf;
	void* gr;
	vec_t eye, scr00, stepx, stepy;
} wparam_t;

static void*
render_worker( void* ptr )
{
	wparam_t* wp = (wparam_t*) ptr;

	/* compute for y's in parallel */
	for( int y=(wp->tid); y<(wp->pconf->res_y); y+=(wp->nth) ) {
		for( int x=0; x<(wp->pconf->res_x); x++ ) {
			vec_t ray = vec_add( vec_add(wp->scr00, vec_scale(x, wp->stepx)),
									vec_scale(y, wp->stepy) );
			color_t c = trace_ray1(wp->pconf, wp->eye, ray, EPS, INFINITY, MAX_RECUR);
			gr_putpixel(wp->gr, x, y, c);
		}
	}
	return ptr;
}

/*---------------*/
/* renderer main */

bool
render( void* gr, conf_t* pconf, int cam, int nth )
{
	if( cam < 0 || cam >= pconf->ncams ) {
		fprintf(stderr,"invalid cam %d\n", cam);
		return false;
	}

	if( nth < 2 ) {
		vec_t scr00, stepx, stepy;
		get_vp_vectors(&pconf->cams[cam], pconf->res_x, pconf->res_y,
						&scr00, &stepx, &stepy);
		vec_t eye = pconf->cams[cam].vp;

		for( int y=0; y < pconf->res_y; y++ ) {
			for( int x=0; x < pconf->res_x; x++ ) {
				vec_t ray = vec_add( vec_add(scr00, vec_scale(x, stepx)),
										vec_scale(y, stepy) );
				color_t c = trace_ray1(pconf, eye, ray, EPS, INFINITY, MAX_RECUR);
				gr_putpixel(gr, x, y, c);
			}
		}
	}
	else { /* multi-threaded */
		bool ok = true;
		wparam_t wp0;
		wp0.nth = nth;
		wp0.pconf = pconf;
		wp0.gr = gr;

		get_vp_vectors(&pconf->cams[cam], pconf->res_x, pconf->res_y,
						&wp0.scr00, &wp0.stepx, &wp0.stepy);
		wp0.eye = pconf->cams[cam].vp;

		/* prepare multi-threading */
		pthread_t* thr = (pthread_t*) malloc(sizeof(pthread_t) * nth);
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		/* do the work */
		wparam_t* wps = (wparam_t*) malloc(sizeof(wparam_t) * nth);
		for( int i=0; i<nth; i++ ) {
			memcpy(&wps[i], &wp0, sizeof(wparam_t));
			wps[i].tid = i;
			if( pthread_create(&thr[i], &attr, render_worker, (void*)(&wps[i])) ) {
				perror("pthread_create");
				ok = false;
				goto bad_thread;
			}
		}
		/* wrap up */
		pthread_attr_destroy(&attr);
		for( int i=0; i<nth; i++ ) {
			void* status;
			if( pthread_join(thr[i], &status) ) {
				perror("pthread_join");
				ok = false;
				goto bad_thread;
			}
		}

	  bad_thread:;
		free(wps);
		free(thr);
		if( !ok ) return false;
	}

	gr_flush(gr); /* update screen */
	return true;
}

/* EOF */
