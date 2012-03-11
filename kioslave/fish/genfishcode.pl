#!/usr/bin/perl
use strict;
use warnings;

use Digest::MD5;

sub md5sum {
    my $filename = shift;
    my $digest;
    eval {
        open( my $FILE, '<', $filename )
            or die "Can't find file $filename\n";
        my $ctx = Digest::MD5->new;
        $ctx->addfile($FILE);
        $digest = $ctx->hexdigest;
        close($FILE);
    };
    if ($@) {
        warn $@;
    }
    return $digest;
}

my $file = $ARGV[0] or die "Missing filename argument";

my $fish_md5 = md5sum($file)
    or die "Couldn't compute MD5 for some reason\n";
print qq{#define CHECKSUM "$fish_md5"\n};
print qq{static const char *fishCode(\n};

open( my $FISH, "<", "$file" ) or die "Can't open $file\n";
while (<$FISH>) {
    chomp;
    s|\\|\\\\|g;
    s|"|\\"|g;
    s/^\s*/"/;
    next if /^"# /;
    s/\s*$/\\n"/;
    next if /^"\\n"$/;
    print "$_\n";
}
close($FISH);
print qq{);\n};
