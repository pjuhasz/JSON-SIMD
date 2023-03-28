#!perl

use strict;
use warnings;
use Test::More;
use Test::MemoryGrowth;
use FindBin qw/$Bin/;
use utf8;

use_ok( 'JSON::XS' );

my $json = '{"method": "handleMessage", "űéúőóüöÁÉ":"púőpóüöúűú日本語\ubaba", "params": ["user1", "we were just talking"], "id": null, "array":[1,11,234,-5,1e5,1e7, true,  false]}';

no_growth {
		my $J = JSON::XS->new->use_simdjson;
		my $perl = $J->decode($json);
	}
	calls   => 5000000,
	burn_in => 10,
	'decode does not leak';

my $J_longlived = JSON::XS->new->use_simdjson;
no_growth {
		my $perl = $J_longlived->decode($json);
	}
	calls   => 5000000,
	burn_in => 10,
	'decode with persistent object does not leak';

no_growth {
		my $J = JSON::XS->new->use_simdjson;
		my $perl = $J->decode_at_path($json, '/params');
	}
	calls   => 5000000,
	burn_in => 10,
	'decode_at_path does not leak';

done_testing();
