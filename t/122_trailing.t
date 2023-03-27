BEGIN { $| = 1; print "1..28\n"; }

use utf8;
use JSON::XS;
no warnings;

our $test;
sub ok($) {
   print $_[0] ? "" : "not ", "ok ", ++$test, "\n";
   return !!$_[0];
}

# more tests with trailing garbage
for (
    [ q/[1, 2] foo/,      'garbage', 7],
    [ q/[1, 2] foo ]/,    'garbage', 7],
    [ q/[1, 2] foo ] /,   'garbage', 7],
    [ q/[1, 2] foo }/,    'garbage', 7],
    [ q/[1, 2} foo ]/,    'improper structure', 7],
    [ q/{"1": 2} foo/,    'garbage', 9],
    [ q/{"1": 2} foo }/,  'garbage', 9],
    [ q/{"1": 2} foo } /, 'garbage', 9],
    [ q/{"1": 2} foo ]/,  'garbage', 9],
    [ q/{"1": 2] foo }/,  'improper structure', 9],
) {
    my ($json, $error, $offset) = @$_;
    my $got_offset; 
    eval { JSON::XS->new->use_simdjson(1)->decode($json) }; 
    ok $@ =~ /$error/
        or warn "# $json   $@";

    eval { (undef, $got_offset) = JSON::XS->new->use_simdjson(1)->decode_prefix($json) };
    if ($error eq 'garbage') {
        ok !$@
            or warn "# $json   $@";;
        ok $offset == $got_offset if $error eq 'garbage'
            or warn "# $json   $@";
    }
}
