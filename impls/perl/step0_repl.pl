#!perl
use strict;
use warnings;
use Term::ReadLine;

sub READ {
    return $_[0];
}
sub EVAL {
    return $_[0];
}
sub PRINT {
    return $_[0];
}

sub rep {
    my $line=shift;
    return PRINT EVAL READ $line;
}

my $line;
my $term=Term::ReadLine->new('mal');
my $prompt='user> ';
my $OUT=$term->OUT || \*STDOUT;
while(defined ($line=$term->readline($prompt))) {
    chomp $line;
    $term->addhistory($line) if $line =~ /\S/;
    print $OUT $line, "\n";
}
