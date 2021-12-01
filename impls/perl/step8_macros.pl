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
        'defmacro!'=>\&eval_defmacro_bang,
        'let*'=>\&eval_let_star,
        do=>\&eval_do,
        if=>\&eval_if,
        'fn*'=>\&eval_fn_star,    
        quote=>\&eval_quote,
        quasiquoteexpand=>\&eval_quasiquoteexpand,
        quasiquote=>\&eval_quasiquote,
        macroexpand=>\&eval_macroexpand,
);

sub READ {
    my $str=shift;
    my $ast=Reader::read_str($str);
    defined $ast or die "\n";
    return $ast;
}
sub EVAL {
    my ($val, $env)=@_;
    $env //= $repl_env;
    $val=macroexpand($val, $env);
    return eval_ast($val, $env) unless ref($val) eq 'MalList' && @$val;
    my $fn=$val->[0];
    if(ref $fn eq 'MalSymbol' && exists $special{$$fn}) {
        return $special{$$fn}->($val, $env)
    }
    return $special{$val->[0]}->($val, $env) if exists $special{$val->[0]};
    my $list=eval_ast($val, $env);
    $fn=shift @$list;
    die "bad function\n" unless ref($fn) eq 'CODE';
    @_=@$list;
    goto &$fn;
}
sub PRINT {
    my $val=shift;
    return Printer::pr_str($val);
}

sub rep {
    my $line=shift;
    return PRINT EVAL READ $line;
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
sub eval_defmacro_bang {
    my ($ast, $env)=@_;
    my $key=$ast->[1];
    my $value=EVAL($ast->[2], $env);
    $value=bless $value, 'MalMacro' if ref $value eq 'CODE'; 
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
    @_=($ast->[2], $inner_env);
    goto &EVAL;
}
sub eval_do {
    my ($ast, $env)=@_;
    my $val='nil';
    my (undef, @args)=@$ast;
    while(@args>1) {
        EVAL(shift @args, $env);
    }
    @_=(shift @args, $env);
    goto &EVAL;
}
sub eval_if {
    my ($ast, $env)=@_;
    my $val=EVAL($ast->[1], $env);
    my $code=($val eq 'nil' || $val eq 'false') ? $ast->[3]//'nil' : $ast->[2];
    @_=($code, $env);
    goto &EVAL;
}
sub eval_fn_star {
    my ($ast, $outer_env)=@_;
    my (undef, $bindings, $code)=@$ast;
    ref $bindings eq 'MalList' or ref $bindings eq 'MalVector' or die "bad fn*\n";
    return sub {
        my $env=$outer_env->extend();
        $env->bind($bindings, \@_);
        @_=($code, $env);
        goto &EVAL;
    };
}

sub eval_quote {
    my $ast=shift;
    return $ast->[1];
}
sub quasiquote {
    my $ast=shift;
    return bless [ Reader::read_atom('quote'), $ast ], 'MalList'
        if ref $ast eq 'MalHash' || ref $ast eq 'MalSymbol';
    return $ast if ref $ast ne 'MalList' && ref $ast ne 'MalVector';    
    return $ast->[1] if ref $ast eq 'MalList' && ref $ast->[0] eq 'MalSymbol' && ${$ast->[0]} eq 'unquote';
    my @ast=@$ast;
    my $list=bless [], 'MalList';
    while(@ast) {
        my $elt=pop @ast;
        if(ref $elt eq 'MalList' && ref $elt->[0] eq 'MalSymbol' && ${$elt->[0]} eq 'splice-unquote') {
            $list=bless [Reader::read_atom('concat'), $elt->[1], $list ], 'MalList';
        } else {
            $list=bless [Reader::read_atom('cons'), quasiquote($elt), $list ], 'MalList';
        }
    }
    return $list if ref $ast eq 'MalList';
    return bless [ Reader::read_atom('vec'), $list], 'MalList';
}
sub eval_quasiquoteexpand {
    my ($ast, $env)=@_;
    return quasiquote($ast->[1]);
}
sub eval_quasiquote {
    my ($ast, $env)=@_;
    @_=(quasiquote($ast->[1]), $env);
    goto &EVAL;
}
sub macroexpand {
    my ($ast, $env)=@_;
    while(ref $ast eq 'MalList'
          and ref $ast->[0] eq 'MalSymbol'
          and my $macroenv=$env->find(${$ast->[0]})) {
        my $macro=$macroenv->get(${$ast->[0]});
        return $ast if ref $macro ne 'MalMacro';
        my (undef, @args)=@$ast;
        $ast=$macro->(@args);
    }
    return $ast;
}
sub eval_macroexpand {
    my ($ast, $env)=@_;
    return macroexpand($ast->[1], $env);
}


my $line;
my $term=Term::ReadLine->new('mal');
my $prompt='user> ';
our $OUT=$term->OUT || \*STDOUT;

rep($_) for @Core::ns;
my $filename=shift @ARGV;
$repl_env->set('*ARGV*',
    bless [
        map { my $arg=$_; bless \$arg, 'MalString' } @ARGV
    ], 'MalList');
if(defined $filename) {
    $repl_env->get('load-file')->(bless \$filename, 'MalString');
    exit;
}

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
