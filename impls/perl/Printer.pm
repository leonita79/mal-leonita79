package Printer;

sub pr_str {
    my $val=shift;
    ref $val or return $val;
    ref $val eq 'MalSymbol' and return $$val;
    ref $val eq 'MalKeyword' and return substr $$val, 1;
    ref $val eq 'MalString' and return $$val;
    ref $val eq 'MalList' and return pr_list('(', $val, ')');
    ref $val eq 'MalVector' and return pr_list('[', $val, ']');
    ref $val eq 'MalHash' and return pr_hash('{', $val, '}');
}

sub pr_list {
    my ($open, $val, $close)=@_;
    return $open . join(' ', map { pr_str($_) } @$val) . $close;
}

sub pr_hash {
    my ($open, $val, $close)=@_;
    my @val_list=map { thaw_key($_), $val->{$_} } keys %$val;
    return pr_list($open, \@val_list, $close);
}

sub thaw_key {
    my $key=shift;
    my $type=($key =~ /^\0/) ? 'MalKeyword' : 'MalString';
    return bless \$key, $type; 
}

l;
