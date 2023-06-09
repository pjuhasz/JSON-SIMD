BEGIN { $| = 1; print "1..10\n"; }

use utf8;
use JSON::SIMD;

our $test;
sub ok($) {
   print $_[0] ? "" : "not ", "ok ", ++$test, "\n";
}

my $json = JSON::SIMD->new->relaxed;

ok ('[1,2,3]' eq encode_json $json->decode (' [1,2, 3]'));
ok ('[1,2,4]' eq encode_json $json->decode ('[1,2, 4 , ]'));
ok (!eval { $json->decode ('[1,2, 3,4,,]') });
ok (!eval { $json->decode ('[,1]') });

ok ('{"1":2}' eq encode_json $json->decode (' {"1":2}'));
ok ('{"1":2}' eq encode_json $json->decode ('{"1":2,}'));
ok (!eval { $json->decode ('{,}') });

ok ('[1,2]' eq encode_json $json->decode ("[1#,2\n ,2,#  ]  \n\t]"));

$json = JSON::SIMD->new->relaxed->use_simdjson(1);
ok ($json->get_use_simdjson == 0);
$json = JSON::SIMD->new->use_simdjson(1)->relaxed;
ok ($json->get_use_simdjson == 0);
