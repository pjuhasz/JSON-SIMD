#! perl

# adapted from a test by Martin Evans

use strict;
use warnings;

use JSON::SIMD;

print "1..1\n";

my $data = ["\x{53f0}\x{6240}\x{306e}\x{6d41}\x{3057}",
            "\x{6c60}\x{306e}\x{30ab}\x{30a8}\x{30eb}"];
my $js = JSON::SIMD->new->encode ($data);
my $j = JSON::SIMD->new->use_simdjson(0);
my $object = $j->incr_parse ($js);

die "no object" if !$object;

eval { $j->incr_text };

print $@ ? "not " : "", "ok 1 # $@\n";

