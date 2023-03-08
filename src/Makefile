INCS =
LIBS = -lSDL2 -lm -lpthread
DEP = Makefile.d

DEFS = #-DDEBUG
CFLAGS = $(DEFS) $(INCS) -Wall -O2

EXE = rt
OBJS = main.o conf.o vec.o gr_sdl.o miscutil.o render.o camera.o
SRCS = $(OBJS:.o=.c)

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

tar:
	tar cvfz rt-`date "+%Y%m%d.%H%M%S"`.tgz Makefile* *.[ch] rt.conf

clean:
	$(RM) $(OBJS)

depend:
	$(CC) $(DEFS) $(INCS) -MM $(SRCS) > $(DEP)

-include $(DEP)
