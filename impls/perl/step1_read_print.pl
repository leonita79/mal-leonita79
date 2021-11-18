#!perl
use strict;
use warnings;
use FindBin;
use lib $FindBin::Bin;
use Term::ReadLine;
use Reader;
use Printer;

sub READ {
    my $str=shift;
    return Reader::read_str($str);
}
sub EVAL {
    my $val=shift;
    return $val;
}
sub PRINT {
    my $val=shift;
    return Printer::pr_str($val);
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
    eval {
        print $OUT rep($line), "\n";
    } or do {
        print $@, "\n" if $@;
    };
}
