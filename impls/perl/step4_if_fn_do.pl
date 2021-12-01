#!perl
use strict;
use warnings;
use FindBin;
use lib $FindBin::Bin;
use Term::ReadLine;
use Reader;
use Printer;
use Env;
use Core;

my $repl_env=Env->new(%Core::ns);

my %special=(
        'def!'=>\&eval_def_bang,
        'let*'=>\&eval_let_star,
        do=>\&eval_do,
        if=>\&eval_if,
        'fn*'=>\&eval_fn_star,    
        );

sub READ {
    my $str=shift;
    my $ast=Reader::read_str($str);
    defined $ast or die "\n";
    return $ast;
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
sub eval_do {
    my ($ast, $env)=@_;
    my $val='nil';
    my (undef, @args)=@$ast;
    while(@args) {
        $val=EVAL(shift @args, $env);
    }
    return $val;
}
sub eval_if {
    my ($ast, $env)=@_;
    my $val=EVAL($ast->[1], $env);
    return EVAL($ast->[3]//'nil', $env) if $val eq 'nil' || $val eq 'false';
    return EVAL($ast->[2], $env);
}
sub eval_fn_star {
    my ($ast, $outer_env)=@_;
    my (undef, $bindings, $code)=@$ast;
    ref $bindings eq 'MalList' or ref $bindings eq 'MalVector' or die "bad fn*\n";
    return sub {
        my $env=$outer_env->extend();
        $env->bind($bindings, \@_);
        return EVAL($code, $env);
    };
}

rep($_) for (
    '(def! not (fn* (a) (if a false true)))',
    '(def! load-file (fn* (f) (eval (read-string (str "(do " (slurp f) "\nnil)")))))',
);

my $line;
my $term=Term::ReadLine->new('mal');
my $prompt='user> ';
our $OUT=$term->OUT || \*STDOUT;
while(defined ($line=$term->readline($prompt))) {
    chomp $line;
    $term->addhistory($line) if $line =~ /\S/;
    eval {
        print $OUT rep($line), "\n";
    } or do {
        chomp $@;
        print $@, "\n" if $@;
    };
}
