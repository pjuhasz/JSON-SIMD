#include <iostream>
#include "simdjson.h"
#include "simdjson_wrapper.h"

using namespace simdjson; // optional

static SV* recursive_parse_json(simdjson_decode_t *dec, ondemand::value element) {
  SV* res = NULL;

  ondemand::json_type t;
  auto err = element.type().get(t);
  if (err) {
    dec->error_code = err;
    return NULL;
  }

  switch (t) {
  case ondemand::json_type::array: 
    {
      AV *av = newAV();

      for (auto child : element.get_array()) {
        ondemand::value val;
        auto err = child.get(val);
        if (err) {
          dec->error_code = err;
          SvREFCNT_dec (av);
          return NULL;
        }
        SV *elem = recursive_parse_json(dec, val);
        if (!elem) {
          SvREFCNT_dec (av);
          return NULL;
        }
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
        if (err) {
          dec->error_code = err;
          SvREFCNT_dec (hv);
          return NULL;
        }
        ondemand::value val;
        err = field.value().get(val);
        if (err) {
          dec->error_code = err;
          SvREFCNT_dec (hv);
          return NULL;
        }
        SV *sv_value = recursive_parse_json(dec, val);
        if (!sv_value) {
          SvREFCNT_dec (hv);
          return NULL;
        }
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
        dec->error_code = err;
        return NULL;
      }
      ondemand::number_type nt = num.get_number_type();
      switch (nt) {
      case ondemand::number_type::floating_point_number:
        {
          double d = 0.0;
          auto err = element.get_double().get(d);
          if (err) {
            dec->error_code = err;
            return NULL;
          }
          res = newSVnv(d);
          break;
        }
      case ondemand::number_type::signed_integer:
        {
          int64_t i = 0;
          auto err = element.get_int64().get(i);
          if (err) {
            dec->error_code = err;
            return NULL;
          }
          res = newSViv((IV)i);
          break;
        }
      case ondemand::number_type::unsigned_integer:
        {
          uint64_t u = 0;
          auto err = element.get_uint64().get(u);
          if (err) {
            dec->error_code = err;
            return NULL;
          }
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
      if (err) {
        dec->error_code = err;
        return NULL;
      }
      res = newSVpvn_utf8(str.data(), str.size(), 1);
      break;
    }
  case ondemand::json_type::boolean:
    {
      bool b = false;
      auto err = element.get_bool().get(b);
      if (err) {
        dec->error_code = err;
        return NULL;
      }
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
  if (valid_location) {
    const char *err_str = "";
    doc.current_location().get(err_str);
    std::cerr << "lofasz error " << dec->error_code << " near " << err_str << std::endl;
  } else {
    std::cerr << "lofasz error " << dec->error_code << " while parsing document" << std::endl;
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
