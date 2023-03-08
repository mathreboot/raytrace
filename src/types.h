#ifndef _types_h
#define _types_h

/*---- basic types ----*/

typedef enum { false=0, true=1 } bool;

typedef struct {
	int r, g, b;
} color_t;

typedef struct {
	double x, y, z;
} vec_t;

/*---- cameras ----*/

typedef struct {
	vec_t vp, ov, up; /* ov and up are normalized */
	double fov; /* in degree */
} camera_t;

/*---- lights ----*/

typedef enum { L_AMB, L_PT } lighttype_t;

typedef struct {
	lighttype_t type;
	double lum;
	color_t col;
	vec_t coord; /* for point-light only */
} light_t;

/*---- various objects ----*/

typedef enum { O_SP=0, O_PL, O_TR, O_SQ, O_CY } objtype_t;

#define  is_planar(t)	((t) == O_PL || (t) == O_TR || (t) == O_SQ)

typedef struct {
	vec_t center;
	double r;
} obj_sp_t;

typedef struct {
	vec_t coord;
} obj_pl_t;

typedef struct {
	vec_t p1, p2, p3;
	vec_t v12, v23, v31;	/* to speed up calc. */
} obj_tr_t;

typedef struct {
	vec_t center, up;
	double side;
	vec_t p1, p2, p3, p4;		/* to speed up calc. */
	vec_t v12, v23, v34, v41;	/* to speed up calc. */
} obj_sq_t;

typedef struct {
	vec_t coord;
	double r, h;
} obj_cy_t;

typedef struct {
	objtype_t type;
	color_t col;
	double shine;
	double refl;
	vec_t ov; /* only for planar objects */
	union {
		obj_sp_t sp;
		obj_pl_t pl;
		obj_sq_t sq;
		obj_cy_t cy;
		obj_tr_t tr;
	} u;
} obj_t;

/*---- configuration ----*/

typedef struct {
	int res_x, res_y;
	int ncams, nlights, nobjs;
	double kamb, kd, ks; /* coeff's for reflection (amb + diffuse + specular = 1) */
	camera_t* cams;
	light_t* lights;
	obj_t* objs;
} conf_t;

#endif
