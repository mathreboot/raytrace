#ifndef _conf_h
#define _conf_h

#include "types.h"

bool conf_load( conf_t*, const char* fname );

void conf_init( conf_t* );
void conf_clear( conf_t* );

void conf_print( conf_t* );

#endif
