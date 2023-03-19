#include <iostream>
#include "simdjson.h"
#include "simdjson_wrapper.h"

using namespace simdjson; // optional

static bool validate_large_number(std::string_view& str) {
  // check if it matches /[+-]?[0-9]+\.?[0-9]*(?:[eE][+-]?[0-9]+)?/, clumsily and slowly
  // TODO
  return true;
}

#define ERROR_RETURN \
  do { \
    if (err) { \
      dec->error_code = err; \
      dec->error_line_number = __LINE__; \
      return NULL; \
    } \
  } while (0)

#define ERROR_RETURN_CLEANUP(var) \
  do { \
    if (err) { \
      dec->error_code = err; \
      dec->error_line_number = __LINE__; \
      SvREFCNT_dec (var); \
      return NULL; \
    } \
  } while (0)

#define NULL_RETURN_CLEANUP(sv, var) \
  do { \
    if (!sv) { \
      SvREFCNT_dec (var); \
      return NULL; \
    } \
  } while (0)

static SV* recursive_parse_json(simdjson_decode_t *dec, ondemand::value element) {
  SV* res = NULL;

  ondemand::json_type t;
  auto err = element.type().get(t);
  ERROR_RETURN;

  switch (t) {
  case ondemand::json_type::array: 
    {
      AV *av = newAV();

      for (auto child : element.get_array()) {
        ondemand::value val;
        auto err = child.get(val);
        ERROR_RETURN_CLEANUP(av);

        SV *elem = recursive_parse_json(dec, val);
        NULL_RETURN_CLEANUP(elem, av);

        av_push(av, elem);
      }

      res = newRV_noinc ((SV *)av);
      break;
    }
  case ondemand::json_type::object:
    {
      HV *hv = newHV();

      for (auto field : element.get_object()) {
        std::string_view key;
        auto err = field.unescaped_key().get(key);
        ERROR_RETURN_CLEANUP(hv);

        ondemand::value val;
        err = field.value().get(val);
        ERROR_RETURN_CLEANUP(hv);

        SV *sv_value = recursive_parse_json(dec, val);
        NULL_RETURN_CLEANUP(sv_value, hv);

        hv_store (hv, key.data(), key.size(), sv_value, 0);
      }

      res = newRV_noinc ((SV *)hv);
      break;
    }
  case ondemand::json_type::number:
    {
      ondemand::number num;
      auto err = element.get_number().get(num);
      if (err) {
        // handle case of large numbers:
        // we save it as a string, but try to validate if it looks like a number at least
        auto str = element.raw_json_token();
        if (!validate_large_number(str)) {
          // we forge an error code and bail out
          err = NUMBER_ERROR;
          ERROR_RETURN;
        }
        res = newSVpvn_utf8(str.data(), str.size(), 1);
        break;
      }

      ondemand::number_type nt = num.get_number_type();
      switch (nt) {
      case ondemand::number_type::floating_point_number:
        {
          double d = 0.0;
          auto err = element.get_double().get(d);
          ERROR_RETURN;

          res = newSVnv(d);
          break;
        }
      case ondemand::number_type::signed_integer:
        {
          int64_t i = 0;
          auto err = element.get_int64().get(i);
          ERROR_RETURN;

          res = newSViv((IV)i);
          break;
        }
      case ondemand::number_type::unsigned_integer:
        {
          uint64_t u = 0;
          auto err = element.get_uint64().get(u);
          ERROR_RETURN;

          res = newSVuv((UV)u);
          break;
        }
      }
      break;
    }
  case ondemand::json_type::string:
    {
      std::string_view str;
      auto err = element.get_string().get(str);
      ERROR_RETURN;

      res = newSVpvn_utf8(str.data(), str.size(), 1);
      break;
    }
  case ondemand::json_type::boolean:
    {
      bool b = false;
      auto err = element.get_bool().get(b);
      ERROR_RETURN;

      res = newSVsv(b ? dec->v_true : dec->v_false);
      break;
    }
  case ondemand::json_type::null:
    bool is_null;
    auto err = element.is_null().get(is_null);
    if(!is_null || err) {
      // XXX Can we ever get here?
      dec->error_code = err;
      return NULL;
    }
    res = newSVsv (&PL_sv_undef);
    break;
  }
  return res;
}

static void print_error(simdjson_decode_t *dec, ondemand::document& doc, bool valid_location) {
  // TODO croak or save error msg or something
  const char *err_msg = error_message((simdjson::error_code)dec->error_code);
  if (valid_location) {
    const char *location = "";
    doc.current_location().get(location); // TODO trim fragment;
    std::cerr << "lofasz error " << err_msg << " at line " << dec->error_line_number << " near " << location << std::endl;
  } else {
    std::cerr << "lofasz error " << err_msg << " at line " << dec->error_line_number << " while parsing document" << std::endl;
  }
}

simdjson_parser_t simdjson_init() {
  return new ondemand::parser;
}

SV * simdjson_decode(simdjson_decode_t *dec) {
  SV *sv;

  char *end = SvEND(dec->input);
  *end = '\0';
  SvGROW(dec->input, SvCUR (dec->input) + SIMDJSON_PADDING);

  ondemand::parser* parser = static_cast<ondemand::parser*>(dec->parser);
  ondemand::document doc;
  auto err = parser->iterate(SvPVX(dec->input), SvCUR(dec->input), SvLEN(dec->input)).get(doc);
  if (err) {
    dec->error_code = err;
    print_error(dec, doc, false);
    return NULL;
  }
  ondemand::value val;
  err = doc.get_value().get(val);
  if (err) {
    // TODO handle scalar case if allow_nonref option is allowed
    dec->error_code = err;
    print_error(dec, doc, false);
    return NULL;
  }
  sv = recursive_parse_json(dec, val);
  if (dec->error_code != 0) {
    print_error(dec, doc, true);
  }
  return sv;
}

void simdjson_destroy(simdjson_parser_t wrapper) {
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
