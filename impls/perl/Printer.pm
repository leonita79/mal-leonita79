package Printer;

sub pr_str {
    my $val=shift;
    ref $val or return $val;
    ref $val eq 'MalSymbol' and return $$val;
    ref $val eq 'MalString' and return $$val;
    ref $val eq 'MalList' and return pr_list('(', $val, ')');
    ref $val eq 'MalVector' and return pr_list('[', $val, ']');
    ref $val eq 'MalHash' and return pr_list('{', $val, '}');
}

sub pr_list {
    my ($open, $val, $close)=@_;
    my @list=map { pr_str($_) } @$val;
    return $open . join(' ', @list) . $close;
}
l;
