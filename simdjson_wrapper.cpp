#include <iostream>
#include "simdjson.h"
#include "simdjson_wrapper.h"

using namespace simdjson; // optional

static SV* recursive_parse_json(ondemand::value element, SV* v_true, SV* v_false) {
  SV* res = NULL;

  switch (element.type()) {
  case ondemand::json_type::array: 
    {
      AV *av = newAV();

      for (auto child : element.get_array()) {
        SV *elem = recursive_parse_json(child.value(), v_true, v_false);
        // FIXME error
        av_push(av, elem);
      }

      res = newRV_noinc ((SV *)av);
      break;
    }
  case ondemand::json_type::object:
    {
      HV *hv = newHV();

      for (auto field : element.get_object()) {
        std::string_view key = field.unescaped_key();
        SV *value = recursive_parse_json(field.value(), v_true, v_false);
        // FIXME error
        hv_store (hv, key.data(), key.size(), value, 0);
      }

      res = newRV_noinc ((SV *)hv);
      break;
    }
  case ondemand::json_type::number:
    switch (element.get_number_type()) {
    case ondemand::number_type::floating_point_number:
      {
        double d = 0.0;
        element.get_double().get(d);
        // FIXME error
        res = newSVnv(d);
        break;
      }
    case ondemand::number_type::signed_integer:
      {
        int64_t i = 0;
        element.get_int64().get(i);
        res = newSViv((IV)i);
        break;
      }
    case ondemand::number_type::unsigned_integer:
      {
        uint64_t u = 0;
        element.get_uint64().get(u);
        res = newSVuv((UV)u);
        break;
      }
    }
    break;
  case ondemand::json_type::string:
    {
      std::string_view str = element.get_string();
      res = newSVpvn_utf8(str.data(), str.size(), 1);
      break;
    }
  case ondemand::json_type::boolean:
    {
      bool b = false;
      element.get_bool().get(b);
      // FIXME error
      if (b) {
        res = newSVsv(v_true);
      } else {
        res = newSVsv(v_false);
      }
      break;
    }
  case ondemand::json_type::null:
    if(element.is_null()) {
      res = newSVsv (&PL_sv_undef);
    }
    // FIXME error
    break;
  }
  return res;
}

Simdjson_wrapper simdjson_init() {
	return new ondemand::parser;
}

SV * simdjson_decode(Simdjson_wrapper wrapper, SV *input, SV* v_true, SV* v_false) {
  SV *sv;

  SvGROW(input, SvCUR (input) + SIMDJSON_PADDING);
  /*char *end = SvEND(input);
  *end = '\0';*/

  ondemand::parser* parser = static_cast<ondemand::parser*>(wrapper);
  ondemand::document doc = parser->iterate(SvPVX(input), SvCUR(input), SvLEN(input));
  ondemand::value val = doc;
  sv = recursive_parse_json(val, v_true, v_false);
  return sv;
}

void simdjson_destroy(Simdjson_wrapper wrapper) {
	ondemand::parser* parser = static_cast<ondemand::parser*>(wrapper);
	delete parser;
}

void simdjson_print_info() {
	std::cout << "simdjson v" << SIMDJSON_VERSION << std::endl;
	std::cout << "Detected the best implementation for your machine: " << simdjson::get_active_implementation()->name();
	std::cout << "(" << simdjson::get_active_implementation()->description() << ")" << std::endl;
	for (auto implementation : simdjson::get_available_implementations()) {
	  std::cout << implementation->name() << ": " << implementation->description() << std::endl;
	}
}
