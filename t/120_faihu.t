#! perl

# adapted from a test by Aristotle Pagaltzis (http://intertwingly.net/blog/2007/11/15/Astral-Plane-Characters-in-Json)

use strict;
use warnings;

use JSON::SIMD;
use Encode qw(encode decode);

use Test::More tests => 3;

my ($faihu, $faihu_json, $roundtrip, $js) = "\x{10346}";

$js = JSON::SIMD->new->allow_nonref->ascii->use_simdjson(0);
$faihu_json = $js->encode($faihu);
$roundtrip = $js->decode($faihu_json);
is ($roundtrip, $faihu, 'JSON in ASCII roundtrips correctly');

$js = JSON::SIMD->new->allow_nonref->utf8->use_simdjson(0);
$faihu_json = $js->encode ($faihu);
$roundtrip = $js->decode ($faihu_json);
is ($roundtrip, $faihu, 'JSON in UTF-8 roundtrips correctly');

$js = JSON::SIMD->new->allow_nonref->use_simdjson(0);
$faihu_json = encode 'UTF-16BE', $js->encode ($faihu);
$roundtrip = $js->decode( decode 'UTF-16BE', $faihu_json);
is ($roundtrip, $faihu, 'JSON with external recoding roundtrips correctly' );
