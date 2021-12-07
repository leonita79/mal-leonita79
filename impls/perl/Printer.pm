package Printer;
use strict;
use warnings;

sub pr_str {
    my ($val, $print_readably)=@_;
    $print_readably //=1;
    ref $val or return $val;
    ref $val eq 'MalSymbol' and return $$val;
    ref $val eq 'MalKeyword' and return substr $$val, 1;
    ref $val eq 'MalString' and return $print_readably ? unwrap_string($val) : $$val;
    ref $val eq 'MalList' and return pr_list('(', $val, ')', $print_readably);
    ref $val eq 'MalVector' and return pr_list('[', $val, ']', $print_readably);
    ref $val eq 'MalHash' and return pr_hash('{', $val, '}', $print_readably);
    ref $val eq 'CODE' and return '#<function>';
    ref $val eq 'MalMacro' and return '#<macro>';
    ref $val eq 'MalAtom' and return '(atom ' . pr_str($$val,$print_readably) . ')'; 
    die "invalid type\n";
}

sub pr_list {
    my ($open, $val, $close, $print_readably)=@_;
    return $open . join(' ', map { pr_str($_, $print_readably) } @$val) . $close;
}

sub pr_hash {
    my ($open, $val, $close, $print_readably)=@_;
    my @val_list=map { thaw_key($_), $val->{$_} } keys %$val;
    return pr_list($open, \@val_list, $close, $print_readably);
}

sub thaw_key {
    my $key=shift;
    my $type=($key =~ /^\0/) ? 'MalKeyword' : 'MalString';
    return bless \$key, $type; 
}

sub unwrap_string {
    my $string=${shift @_};
    $string =~ s/\\/\\\\/g;
    $string =~ s/\n/\\n/g;
    $string =~ s/"/\\"/g;
    return qq{"$string"};
}

1;
