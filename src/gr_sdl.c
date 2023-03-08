#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "defs.h"
#include "gr_sdl.h"

/*--- SDL specific data ---*/

typedef struct {
	SDL_Window* win;
	SDL_Surface* scr;
} sdl_t;

/*--- for keymap sort/search ---*/

typedef struct {
	int first, second;
} intpair_t;

static int
cmp_keymap( const void* a, const void* b )
{
	return ((const intpair_t*)a)->first - ((const intpair_t*)b)->first;
}

static intpair_t keymap[] = {
	{ SDLK_ESCAPE, KEY_ESC },
	{ SDLK_0, KEY_0 },
	{ SDLK_1, KEY_1 },
	{ SDLK_2, KEY_2 },
	{ SDLK_3, KEY_3 },
	{ SDLK_4, KEY_4 },
	{ SDLK_5, KEY_5 },
	{ SDLK_6, KEY_6 },
	{ SDLK_7, KEY_7 },
	{ SDLK_8, KEY_8 },
	{ SDLK_9, KEY_9 },
	{ SDLK_w, KEY_W },
	{ SDLK_a, KEY_A },
	{ SDLK_s, KEY_S },
	{ SDLK_d, KEY_D },
	{ SDLK_q, KEY_Q },
	{ SDLK_e, KEY_E },
	{ SDLK_UP, KEY_UP },
	{ SDLK_DOWN, KEY_DOWN },
	{ SDLK_LEFT, KEY_LEFT },
	{ SDLK_RIGHT, KEY_RIGHT },
	{ SDLK_DELETE, KEY_DEL },
	{ SDLK_PAGEDOWN, KEY_PGDN },
	{ SDLK_SPACE, KEY_SPC },
};

/*--- init & finish ---*/

void*
gr_init( int w, int h, const char* title )
{
	sdl_t* sdl = (sdl_t*) malloc(sizeof(sdl_t));

	if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return NULL;
	}
	sdl->win = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);

	if( sdl->win == NULL ) {
		fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
		free(sdl);
		return NULL;
	}
	sdl->scr = SDL_GetWindowSurface(sdl->win);
	qsort(keymap, sizeof(keymap)/sizeof(intpair_t), sizeof(intpair_t), cmp_keymap);
	return sdl;
}

void
gr_finish( void* ptr )
{
	sdl_t* sdl = (sdl_t*) ptr;
	if( sdl->win ) SDL_DestroyWindow(sdl->win);
	SDL_Quit();
	free(sdl);
}

/*--- some graphics stuff ---*/

void
gr_flush( void* ptr )
{
	SDL_Window* win = ((sdl_t*) ptr)->win;
	SDL_UpdateWindowSurface(win);
}

int
gr_clear( void* ptr, color_t c )
{
	SDL_Surface* scr = ((sdl_t*) ptr)->scr;
	return SDL_FillRect(scr, NULL, SDL_MapRGB(scr->format, c.r, c.g, c.b));
}

int
gr_save_bmp( void* ptr, char* fname )
{
	SDL_Surface* scr = ((sdl_t*) ptr)->scr;
	return SDL_SaveBMP(scr, fname);
}

int
gr_putpixel( void* ptr, int x, int y, color_t c )
{
	SDL_Surface* scr = ((sdl_t*) ptr)->scr;
	if( x < 0 || x >= scr->w || y < 0 || y >= scr->h ) return -1;

	int bpp = scr->format->BytesPerPixel;
	Uint8* p = (Uint8*)(scr->pixels) + (y * scr->pitch) + (x * bpp);

	*((Uint32*)p) = SDL_MapRGB(scr->format, c.r, c.g, c.b);
	return 0;
}

/*--- keyboard stuff ---*/

int
gr_readkey( void* ptr )	/* ptr not used */
{
	SDL_Event e;
	for(;;) {
		SDL_WaitEvent(&e);
		if( e.type == SDL_QUIT ) return -1;
		if( e.type == SDL_KEYDOWN ) break;
		/* ignore other event types */
	}
	intpair_t k;
	k.first = e.key.keysym.sym;
	int nkeys = sizeof(keymap)/sizeof(intpair_t);

	intpair_t* p = bsearch(&k, keymap, nkeys, sizeof(intpair_t), cmp_keymap);
	return (p == NULL) ? KEY_OTHER : p->second;
}

/* EOF */
