#ifndef SIMDJSON_DECODE_H
#define SIMDJSON_DECODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"


typedef void* simdjson_parser_t;

typedef struct simdjson_decode_s {
	simdjson_parser_t parser;
	SV* input;
	SV* v_true;
	SV* v_false;
	int error_code;
} simdjson_decode_t;

simdjson_parser_t simdjson_init();
void simdjson_destroy(simdjson_parser_t self);
SV* simdjson_decode(simdjson_decode_t *dec);

#ifdef __cplusplus
}
#endif

#endif
