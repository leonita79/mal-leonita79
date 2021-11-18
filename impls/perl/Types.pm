package Types;

my %interned_symbol;
sub intern_symbol {
    my $symbol=shift;
    my $class=($symbol =~ /^:/) ? 'MalKeyword' : 'MalSymbol';
    return $interned_symbol{$symbol} ||=bless \$symbol, $class;
}

sub freeze_key {
    my $key=shift;
    ref $key eq 'MalKeyword' and return $$key;
    ref $key eq 'MalString' and return $$key;
    return;
}

1;
