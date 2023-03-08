#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "types.h"
#include "vec.h"
#include "conf.h"

/*---------------------------------*/

static bool
da_append( void** arr, int* cnt, void* item, size_t sz )
{
	if( *cnt == 0 ) *arr = (void*) malloc(sz);
	else *arr = (void*) realloc(*arr, sz * (*cnt+1));
	if( *arr == NULL ) {
		perror("da_append");
		return false;
	}
	memcpy( *arr+(*cnt)*sz, item, sz );
	(*cnt)++;
	return true;
}

/*---------------------------------*/

static bool
validate_01( double d )
{
	return (d < 0 || (d-1.) > 0) ? false : true;
}

static bool
validate_color( const color_t* c )
{
	return (c->r >= 0 && c->r <= 255)
		&& (c->g >= 0 && c->g <= 255)
		&& (c->b >= 0 && c->b <= 255);
}

/*---------------------------------*/

static bool
set_res_conf( conf_t* p, const char* s )
{
	if( sscanf(s," %d %d", &p->res_x, &p->res_y) != 2 ) return false;
	if( p->res_x < 1 || p->res_y < 1 ) return false;
	return true;
}

static bool
set_kd_conf( conf_t* p, const char* s )
{
	if( sscanf(s," %lf", &p->kd) != 1 ) return false;
	if( !validate_01(p->kd) ) return false;
	return true;
}

static bool
add_amb_light_conf( conf_t* p, const char* s )
{
	for( int i=0; i < p->nlights; i++ ) {
		if( p->lights[i].type == L_AMB ) {
			fprintf(stderr,"ambient light: already configured\n");
			return false;
		}
	}

	light_t light;
	light.type = L_AMB;
	if( sscanf(s," %lf %d,%d,%d", &light.lum,
			&light.col.r, &light.col.g, &light.col.b) != 4 ) return false;
	if( !validate_01(light.lum) || !validate_color(&light.col) ) return false;

	p->kamb = light.lum;

	return da_append((void**)&(p->lights), &(p->nlights), &light, sizeof(light_t));
}

static bool
add_pt_light_conf( conf_t* p, const char* s )
{
	light_t light;
	light.type = L_PT;
	if( sscanf(s," %lf,%lf,%lf %lf %d,%d,%d",
			&light.coord.x, &light.coord.y, &light.coord.z, 
			&light.lum,
			&light.col.r, &light.col.g, &light.col.b) != 7 ) return false;
	if( !validate_01(light.lum) || !validate_color(&light.col) ) return false;

	return da_append((void**)&(p->lights), &(p->nlights), &light, sizeof(light_t));
}

static bool
add_camera_conf( conf_t* p, const char* s )
{
	camera_t cam;
	vec_t v;
	if( sscanf(s," %lf,%lf,%lf %lf,%lf,%lf %lf",
			&cam.vp.x, &cam.vp.y, &cam.vp.z,
			&v.x, &v.y, &v.z, &cam.fov) != 7 ) return false;
	if( cam.fov < 0 || cam.fov > 180 ) return false;

	/* setting the 'up' direction */
	vec_t side = vec_cross(v, vec_new(0, 1, 0)); /* default hint is 'y-direction' */
	if( is_zero(vec_len(side)) ) {				/* if looking exactly up/downwards, */
		side = vec_cross(v, vec_new(1, 0, 0));	/* use 'x-direction' instead */
	}
	cam.up = vec_normalize(vec_cross(side, v));
	cam.ov = vec_normalize(v);

	return da_append((void**)&(p->cams), &(p->ncams), &cam, sizeof(camera_t));
}

static bool
add_sphere_conf( conf_t* p, const char* s )
{
	obj_t obj;
	obj.type = O_SP;
	if( sscanf(s," %lf,%lf,%lf %lf %d,%d,%d %lf %lf",
			&obj.u.sp.center.x, &obj.u.sp.center.y, &obj.u.sp.center.z, 
			&obj.u.sp.r,
			&obj.col.r, &obj.col.g, &obj.col.b,
			&obj.shine, &obj.refl) != 9 ) return false;

	if( !validate_color(&obj.col) ) return false;
	if( !validate_01(obj.refl) ) return false;

	return da_append((void**)&(p->objs), &(p->nobjs), &obj, sizeof(obj_t));
}

static bool
add_plane_conf( conf_t* p, const char* s )
{
	obj_t obj;
	obj.type = O_PL;
	vec_t v;
	if( sscanf(s," %lf,%lf,%lf %lf,%lf,%lf %d,%d,%d %lf %lf",
			&obj.u.pl.coord.x, &obj.u.pl.coord.y, &obj.u.pl.coord.z, 
			&v.x, &v.y, &v.z,
			&obj.col.r, &obj.col.g, &obj.col.b,
			&obj.shine, &obj.refl) != 11 ) return false;

	if( !validate_color(&obj.col) ) return false;
	if( !validate_01(obj.refl) ) return false;

	obj.ov = vec_normalize(v);

	return da_append((void**)&(p->objs), &(p->nobjs), &obj, sizeof(obj_t));
}

static bool
add_triangle_conf( conf_t* p, const char* s )
{
	obj_t obj;
	obj.type = O_TR;
	if( sscanf(s," %lf,%lf,%lf %lf,%lf,%lf %lf,%lf,%lf %d,%d,%d %lf %lf",
			&obj.u.tr.p1.x, &obj.u.tr.p1.y, &obj.u.tr.p1.z,
			&obj.u.tr.p2.x, &obj.u.tr.p2.y, &obj.u.tr.p2.z,
			&obj.u.tr.p3.x, &obj.u.tr.p3.y, &obj.u.tr.p3.z,
			&obj.col.r, &obj.col.g, &obj.col.b,
			&obj.shine, &obj.refl) != 14 ) return false;

	if( !validate_color(&obj.col) ) return false;
	if( !validate_01(obj.refl) ) return false;

	/* some pre-calc's */
	obj.u.tr.v12 = vec_sub(obj.u.tr.p2, obj.u.tr.p1);
	obj.u.tr.v23 = vec_sub(obj.u.tr.p3, obj.u.tr.p2);
	obj.u.tr.v31 = vec_sub(obj.u.tr.p1, obj.u.tr.p3);
	obj.ov = vec_cross(obj.u.tr.v12, obj.u.tr.v31);

	return da_append((void**)&(p->objs), &(p->nobjs), &obj, sizeof(obj_t));
}

static bool
add_square_conf( conf_t* p, const char* s )
{
	obj_t obj;
	obj.type = O_SQ;
	vec_t v, t;
	if( sscanf(s," %lf,%lf,%lf %lf,%lf,%lf %lf,%lf,%lf %lf %d,%d,%d %lf %lf",
			&obj.u.sq.center.x, &obj.u.sq.center.y, &obj.u.sq.center.z, 
			&v.x, &v.y, &v.z,
			&t.x, &t.y, &t.z, &obj.u.sq.side,
			&obj.col.r, &obj.col.g, &obj.col.b,
			&obj.shine, &obj.refl) != 15 ) return false;

	if( !validate_color(&obj.col) ) return false;
	if( !validate_01(obj.refl) ) return false;

	obj.ov = vec_normalize(v);
	obj.u.sq.up = vec_normalize(vec_cross(vec_cross(v, t), v));

	/* some pre-calc's */
	obj.u.sq.v12 = vec_scale(obj.u.sq.side, obj.u.sq.up);
	obj.u.sq.v23 = vec_scale(obj.u.sq.side, vec_cross(obj.u.sq.up, obj.ov));
	obj.u.sq.v34 = vec_inv(obj.u.sq.v12);
	obj.u.sq.v41 = vec_inv(obj.u.sq.v23);
	obj.u.sq.p1 = vec_sub(obj.u.sq.center,
						vec_scale(0.5, vec_add(obj.u.sq.v12, obj.u.sq.v23)));
	obj.u.sq.p2 = vec_add(obj.u.sq.p1, obj.u.sq.v12);
	obj.u.sq.p3 = vec_add(obj.u.sq.p2, obj.u.sq.v23);
	obj.u.sq.p4 = vec_add(obj.u.sq.p3, obj.u.sq.v34);

	return da_append((void**)&(p->objs), &(p->nobjs), &obj, sizeof(obj_t));
}

static bool
add_cylinder_conf( conf_t* p, const char* s )
{
	obj_t obj;
	obj.type = O_CY;
	vec_t v;
	if( sscanf(s," %lf,%lf,%lf %lf,%lf,%lf %lf %lf %d,%d,%d %lf %lf",
			&obj.u.cy.coord.x, &obj.u.cy.coord.y, &obj.u.cy.coord.z, 
			&v.x, &v.y, &v.z, &obj.u.cy.r, &obj.u.cy.h,
			&obj.col.r, &obj.col.g, &obj.col.b,
			&obj.shine, &obj.refl) != 13 ) return false;

	if( !validate_color(&obj.col) ) return false;
	if( !validate_01(obj.refl) ) return false;

	obj.ov = vec_normalize(v);

	return da_append((void**)&(p->objs), &(p->nobjs), &obj, sizeof(obj_t));
}

/*---------------------------------*/

#define  MAXCMD		8

typedef struct {
	char cmd[MAXCMD];
	bool (*func)(conf_t*, const char*);
} funcmap_t;

static funcmap_t fmap[] = {
	{ "R",	set_res_conf },
	{ "kd",	set_kd_conf },
	{ "A",	add_amb_light_conf },
	{ "l",	add_pt_light_conf },
	{ "c",	add_camera_conf },
	{ "sp",	add_sphere_conf },
	{ "pl",	add_plane_conf },
	{ "sq",	add_square_conf },
	{ "cy",	add_cylinder_conf },
	{ "tr",	add_triangle_conf },
};

static int
cmp_funcmap( const void* a, const void* b )
{
	return strcmp( ((const funcmap_t*)a)->cmd, ((const funcmap_t*)b)->cmd );
}

/*---------------------------------*/

void
conf_init( conf_t* p )
{
	p->res_x = p->res_y = 0;
	p->ncams = p->nlights = p->nobjs = 0;
	p->kamb = 0;
	p->kd = p->ks = 0.5;
	p->cams = NULL;
	p->lights = NULL;
	p->objs = NULL;
	qsort(fmap, sizeof(fmap)/sizeof(funcmap_t), sizeof(funcmap_t), cmp_funcmap);
}

bool
conf_load( conf_t* pconf, const char* fname )
{
	FILE* fp;
	char* line=NULL;
	size_t linecap=0;
	ssize_t nr;

	if( (fp = fopen(fname,"r")) == NULL ) {
		fprintf(stderr,"conf_load: %s: cannot open\n", fname);
		return false;
	}
	conf_init(pconf);

	int nfuncs = sizeof(fmap)/sizeof(funcmap_t);
	funcmap_t key;
	bool ok = true;

	while( ok && (nr = getline(&line, &linecap, fp)) > 0 ) {
		if( line[0] == '#' ) continue;
		if( strspn(line, " \t\n") == nr ) continue;

		size_t p = strcspn(line, " \t");
		if( p == 0 ) {
			fprintf(stderr,"conf_load: bad input: %s\n", line);
			ok = false;
			break;
		}
		memset(key.cmd, 0, MAXCMD);
		strncpy(key.cmd, line, min(p,MAXCMD-1));
		funcmap_t* f = bsearch(&key, fmap, nfuncs, sizeof(funcmap_t), cmp_funcmap);
		if( f ) {
			if( (ok = (f->func)(pconf, line+p)) == false )
				fprintf(stderr,"conf_load: bad configuration: %s\n", line);
		}
		else {
			fprintf(stderr,"conf_load: unknown command '%s'\n", key.cmd);
			ok = false;
		}
	}

	if( pconf->nlights < 1 || pconf->ncams < 1 || pconf->nobjs < 1 ) {
		fprintf(stderr,"conf_load: light/cam/obj missing\n");
		ok = false;
	}
	if( (pconf->kamb + pconf->kd - 1.) > 0 ) {
		fprintf(stderr,"conf_load: invalid refl. coeff's (ambient + diffuse > 1)\n");
		ok = false;
	}
	else {
		pconf->ks = (1. - pconf->kamb - pconf->kd);
	}
	free(line);
	fclose(fp);
	return ok;
}

void
conf_clear( conf_t* p )
{
	if( p->ncams > 0 ) free(p->cams);
	if( p->nlights > 0 ) free(p->lights);
	if( p->nobjs > 0 ) free(p->objs);
	conf_init(p);
}

/*---------------------------------*/

static void
pr_col( const color_t* c, const char* name )
{
	printf("%s=(%d, %d, %d) ", name, c->r, c->g, c->b);
}

static void
pr_num( double d, const char* name )
{
	printf("%s=%lg ", name, d);
}

static void
print_cam( const camera_t* pc )
{
	vec_print(&pc->vp, "vp");
	vec_print(&pc->ov, "ov");
	vec_print(&pc->up, "up");
	pr_num(pc->fov, "fov");
	putchar('\n');
}

static void
print_light( const light_t* pl )
{
	printf("<%s> ", ((pl->type == L_AMB)? "amb":"pt"));
	if( pl->type == L_AMB ) {
		pr_num(pl->lum, "lum");
		pr_col(&pl->col, "color");
	}
	else {
		vec_print(&pl->coord, "coord");
		pr_num(pl->lum, "lum");
		pr_col(&pl->col, "color");
	}
	putchar('\n');
}

static void
print_obj( const obj_t* po )
{
	/* O_SP=0, O_PL, O_TR, O_SQ, O_CY */
	static const char* objnames[] = { "sp", "pl", "tr", "sq", "cy" };

	printf("<%s> ", objnames[po->type]);
	pr_col(&po->col, "color");
	pr_num(po->shine, "shine");
	pr_num(po->refl, "refl");
	if( is_planar(po->type) )
		vec_print(&po->ov, "ov");

	switch( po->type ) {
		case O_SP:
			vec_print(&po->u.sp.center, "center");
			pr_num(po->u.sp.r, "r");
			break;
		case O_PL:
			vec_print(&po->u.pl.coord, "coord");
			break;
		case O_TR:
			vec_print(&po->u.tr.p1, "p1");
			vec_print(&po->u.tr.p2, "p2");
			vec_print(&po->u.tr.p3, "p3");
			break;
		case O_SQ:
			vec_print(&po->u.sq.center, "center");
			vec_print(&po->u.sq.up, "up");
			pr_num(po->u.sq.side, "side");
			break;
		case O_CY:
			vec_print(&po->u.cy.coord, "coord");
			pr_num(po->u.cy.r, "r");
			pr_num(po->u.cy.h, "h");
			break;
		default:
			break; /* not possible */
	}
	putchar('\n');
}

void
conf_print( conf_t* p )
{
	int i;
	printf("screen: %d * %d\n", p->res_x, p->res_y);
	printf("ambient=%lg, diffuse=%lg, specular=%lg\n", p->kamb, p->kd, p->ks);

	printf("%d camera(s)\n", p->ncams);
	for( i=0; i<p->ncams; i++ ) {
		printf(" camera %d: ", i);
		print_cam(&(p->cams[i]));
	}

	printf("%d light(s)\n", p->nlights);
	for( i=0; i<p->nlights; i++ ) {
		printf(" light  %d: ", i);
		print_light(&(p->lights[i]));
	}

	printf("%d object(s)\n", p->nobjs);
	for( i=0; i<p->nobjs; i++ ) {
		printf(" object %d: ", i);
		print_obj(&(p->objs[i]));
	}
	putchar('\n');
}

/* EOF */
