BEGIN { $| = 1; print "1..30\n"; }

use utf8;
use JSON::XS;
no warnings;

our $test;
sub ok($) {
   print $_[0] ? "" : "not ", "ok ", ++$test, "\n";
   return !!$_[0];
}

sub test {
	my $J = shift;

	my $obj = '{
		"foo" : [
			{ "bar": "baz" },
			[ 1, 2, 3 ],
			null
		],
		"quux": true,
		"~/path": "xxx"
	}';

	my $arr = '[1, {"a": 2}, 3]';

	ok $J->decode_at_pointer($obj, '/foo/1/1') == 1;
	ok $J->decode_at_pointer($obj, '/foo/0/bar') eq "baz";

	my $s = $J->decode_at_pointer($obj, '/foo/0');
	ok ref $s eq 'HASH' and exists $s->{bar};

	ok $J->decode_at_pointer($obj, '/~0~1path') eq 'xxx';

	eval {$J->decode_at_pointer($obj, '/nonexistent/1');}; ok $@ =~ /JSON field referenced does not exist/;
	eval {$J->decode_at_pointer($obj, 'missing /');}; ok $@ =~ /Invalid JSON pointer syntax/;

	ok $J->decode_at_pointer($arr, '/1/a') == 2;
	eval {$J->decode_at_pointer($arr, '/5');}; ok $@ =~ /Attempted to access an element of a JSON array/;
	eval {$J->decode_at_pointer($arr, '/foo');}; ok $@ =~ /The JSON element does not have the requested type/;

	eval {$J->decode_at_pointer($arr, '/-');}; ok $@ =~ /Attempted to access an element of a JSON array/;
	eval {$J->decode_at_pointer($arr, '/001');}; ok $@ =~ /Invalid JSON pointer syntax/;
	eval {$J->decode_at_pointer($arr, '/');}; ok $@ =~ /Invalid JSON pointer syntax/;

	ok $J->decode_at_pointer('1111', '') == 1111;
	eval {$J->decode_at_pointer('1111', '/bar');}; ok $@ =~ /Invalid JSON pointer syntax/;
}

test(JSON::XS->new->use_simdjson);
test(JSON::XS->new);
