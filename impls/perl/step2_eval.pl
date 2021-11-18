#!perl
use strict;
use warnings;
use FindBin;
use lib $FindBin::Bin;
use Term::ReadLine;
use Reader;
use Printer;

my $repl_env={
    '+'=>sub { my $val=0; $val+=shift while @_; return $val; },
    '-'=>sub { my $val=shift; $val-=shift while @_; return $val; },
    '*'=>sub { my $val=1; $val*=shift while @_; return $val; },
    '/'=>sub { my $val=shift; $val=int($val/shift) while @_; return $val; },
};
sub READ {
    my $str=shift;
    return Reader::read_str($str);
}
sub EVAL {
    my ($val, $env)=@_;
    return eval_ast($val, $env) unless ref($val) eq 'MalList' && @$val;
    my $list=eval_ast($val, $env);
    my $fn=shift @$list;
    return $fn->(@$list) if ref($fn) eq 'CODE';
    die "bad function\n";
}
sub PRINT {
    my $val=shift;
    return Printer::pr_str($val);
}

sub rep {
    my $line=shift;
    return PRINT(EVAL(READ($line), $repl_env));
}

sub eval_ast {
    my ($ast, $env)=@_;
    if(ref($ast) eq 'MalSymbol') {
        return $env->{$$ast} if exists $env->{$$ast};
        die "$$ast not defined\n";
    } elsif(ref($ast) eq 'MalList' || ref($ast) eq 'MalVector') {
        return bless [ map { EVAL($_, $env) } @$ast ], ref($ast);
    } elsif(ref($ast) eq 'MalHash') {
        return bless { map { EVAL($_, $env) } %$ast }, ref($ast);
    } else {
        return $ast;
    }
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
