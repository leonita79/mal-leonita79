#!perl
use strict;
use warnings;
use FindBin;
use lib $FindBin::Bin;
use Term::ReadLine;
use Reader;
use Printer;
use Env;

my $repl_env=Env->new(
    '+'=>sub { my $val=0; $val+=shift while @_; return $val; },
    '-'=>sub { my $val=shift; $val-=shift while @_; return $val; },
    '*'=>sub { my $val=1; $val*=shift while @_; return $val; },
    '/'=>sub { my $val=shift; $val=int($val/shift) while @_; return $val; },
);
my %special=(
    'def!'=>\&eval_def_bang,
    'let*'=>\&eval_let_star,
);

sub READ {
    my $str=shift;
    return Reader::read_str($str);
}
sub EVAL {
    my ($val, $env)=@_;
    return eval_ast($val, $env) unless ref($val) eq 'MalList' && @$val;
    my $fn=$val->[0];
    if(ref $fn eq 'MalSymbol' && exists $special{$$fn}) {
        return $special{$$fn}->($val, $env)
    }
    return $special{$val->[0]}->($val, $env) if exists $special{$val->[0]};
    my $list=eval_ast($val, $env);
    $fn=shift @$list;
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
        return $env->get($$ast);
    } elsif(ref($ast) eq 'MalList' || ref($ast) eq 'MalVector') {
        return bless [ map { EVAL($_, $env) } @$ast ], ref($ast);
    } elsif(ref($ast) eq 'MalHash') {
        return bless { map { EVAL($_, $env) } %$ast }, ref($ast);
    } else {
        return $ast;
    }
}

sub eval_def_bang {
    my ($ast, $env)=@_;
    my $key=$ast->[1];
    my $value=EVAL($ast->[2], $env);
    $env->set($key=>$value);
    return $value;
}
sub eval_let_star {
    my ($ast, $env)=@_;
    my $bindings=$ast->[1];
    die "malformed let*\n" if ref($bindings) ne 'MalList' and ref($bindings) ne 'MalVector';
    die "malformed let*\n" if @$bindings % 2;
    my $inner_env=$env->extend();
    while(@$bindings) {
        my $key=shift @$bindings;
        my $value=EVAL(shift @$bindings, $inner_env);
        $inner_env->set($key, $value); 
    }
    return EVAL($ast->[2], $inner_env);
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
