BEGIN { $| = 1; print "1..68\n"; }

use utf8;
use JSON::XS;
no warnings;

our $test;
sub ok($) {
   print $_[0] ? "" : "not ", "ok ", ++$test, "\n";
   return !!$_[0];
}

# more tests with trailing garbage
# the good old legacy parser naturally and trivially detects all these cases correctly,
# however, with simdjson we have to test the various dodgy workarounds :(
for (
    [ q/"a" foo/,           'garbage', 3],
    [ q/"a" "b"/,           'garbage', 3],
    [ q/111 foo/,           'garbage', 3],
    [ q/true foo/,          'garbage', 4],
    [ q/false foo/,         'garbage', 5],
    [ q/null foo/,          'garbage', 4],
    [ q/[1, 2] foo/,        'garbage', 7],
    [ q/[1, 2] foo ]/,      'garbage', 7],
    [ q/[1, 2] foo ] /,     'garbage', 7],
    [ q/[1, 2] foo }/,      'garbage', 7],
    [ q/[1, 2} foo ]/,      'improper structure', 7],
    [ q/[1, 2] [3]/,        'garbage', 7],
    [ q/[1, 2] [/,          'garbage', 7],
    [ q/[1, 2] 111/,        'garbage', 7],
    [ q/[1, 2] "str"/,      'garbage', 7],
    [ q/{"1": 2} foo/,      'garbage', 9],
    [ q/{"1": 2} foo }/,    'garbage', 9],
    [ q/{"1": 2} foo } /,   'garbage', 9],
    [ q/{"1": 2} foo ]/,    'garbage', 9],
    [ q/{"1": 2] foo }/,    'improper structure', 9],
    [ q/{"1": 2} {"3": 4}/, 'garbage', 9],
    [ q/{"1": 2} {/,        'garbage', 9],
    [ q/{"1": 2} 111/,      'garbage', 9],
    [ q/{"1": 2} "str"/,    'garbage', 9],

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
