#ifndef SIMDJSON_DECODE_H
#define SIMDJSON_DECODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"


typedef void* Simdjson_wrapper;

Simdjson_wrapper simdjson_init();
void simdjson_destroy(Simdjson_wrapper self);
SV* simdjson_decode(Simdjson_wrapper self, SV *input, SV* v_true, SV* v_false);

#ifdef __cplusplus
}
#endif

#endif
