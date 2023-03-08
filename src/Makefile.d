main.o: main.c defs.h conf.h types.h render.h miscutil.h gr_sdl.h \
 camera.h
conf.o: conf.c defs.h types.h vec.h conf.h
vec.o: vec.c defs.h vec.h types.h
gr_sdl.o: gr_sdl.c defs.h gr_sdl.h types.h
miscutil.o: miscutil.c defs.h types.h miscutil.h
render.o: render.c defs.h conf.h types.h gr_sdl.h vec.h miscutil.h \
 render.h
camera.o: camera.c vec.h types.h camera.h
