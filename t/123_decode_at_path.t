BEGIN { $| = 1; print "1..10\n"; }

use utf8;
use JSON::XS;
no warnings;

our $test;
sub ok($) {
   print $_[0] ? "" : "not ", "ok ", ++$test, "\n";
   return !!$_[0];
}

my $J = JSON::XS->new->use_simdjson;

my $obj = '{
	"foo" : [
		{ "bar": "baz" },
		[ 1, 2, 3 ],
		null
	],
	"quux": true
}';

my $arr = '[1, {"a": 2}, 3]';

ok $J->decode_at_path($obj, '/foo/1/1') == 1;
ok $J->decode_at_path($obj, '/foo/0/bar') eq "baz";

my $s = $J->decode_at_path($obj, '/foo/0');
ok ref $s eq 'HASH' and exists $s->{bar};

eval {$J->decode_at_path($obj, '/nonexistent/1');}; ok $@ =~ /JSON field referenced does not exist/;
eval {$J->decode_at_path($obj, 'missing /');}; ok $@ =~ /Invalid JSON pointer syntax/;

ok $J->decode_at_path($arr, '/1/a') == 2;
eval {$J->decode_at_path($arr, '/5');}; ok $@ =~ /Attempted to access an element of a JSON array/;
eval {$J->decode_at_path($arr, '/foo');}; ok $@ =~ /The JSON element does not have the requested type/;

ok $J->decode_at_path('1111', '') == 1111;
eval {$J->decode_at_path('1111', '/bar');}; ok $@ =~ /Invalid JSON pointer syntax/;