package Core;

sub mal_bool { return shift ? 'true' : 'false'; }
sub mal_equal {
    my ($first, $second)=@_;
    if(ref($first) eq 'MalList' || ref($first) eq 'MalVector'
        and ref($second) eq 'MalList' || ref($second) eq 'MalVector') {
        scalar @$first==scalar @$second or return;
        for(keys @$first) {
            mal_equal($first->[$_], $second->[$_]) or return;
        }
        return 1;
    }
    ref($first) eq ref($second) or return;
    if(ref($first) eq 'MalHash') {
        scalar keys %$first==scalar keys %$second or return;
        for(keys %$first) {
            mal_equal($first->{$_}, $second->{$_}) or return;
        }
        return 1;
    } elsif(!ref($first)) {
        $first eq $second and return 1;
        return $first==$second;
    } else {
        return $$first eq $$second;
    }
}
our %ns=(
    '+'=>sub { my $val=0; $val+=shift while @_; return $val; },
    '-'=>sub { my $val=shift; $val-=shift while @_; return $val; },
    '*'=>sub { my $val=1; $val*=shift while @_; return $val; },
    '/'=>sub { my $val=shift; $val=int($val/shift) while @_; return $val; },
    'pr-str'=>sub {
        my $str=join ' ', map { Printer::pr_str($_) } @_;
        return bless \$str, 'MalString';
        return Reader::wrap_string('"' . join(" ", @str) . '"');
    },
    str=>sub {
        my $str=join '', map { Printer::pr_str($_, 0) } @_;
        return bless \$str, 'MalString';
    },
    prn=>sub {
        my @str=map { Printer::pr_str($_) } @_;
        print $::OUT join(" ", @str), "\n";
        return 'nil'
    },
    println=>sub {
        my @str=map { Printer::pr_str($_, 0) } @_;
        print $::OUT join(" ", @str), "\n";
        return 'nil'
    },
    list=>sub { bless [@_], 'MalList' },
    'list?'=>sub { mal_bool(ref(shift) eq 'MalList'); },
    'empty?'=>sub {
        my $val=shift;
        mal_bool(ref($val) eq 'MalList' || ref($val) eq 'MalVector' and !@$val);
    },
    count=>sub {
        my $val=shift;
        return 0 if $val eq 'nil';
        ref($val) eq 'MalList' or ref($val) eq 'MalVector' or die "can't count non-list\n";
        return scalar @$val;
    },
    '='=>sub { return mal_bool(mal_equal(@_)); },
    '<'=>sub { 
        my ($first, $second)=@_;
        ref(first) || ref($second) and die "can't compare non-number\n";
        mal_bool($first<$second);
    },
    '>'=>sub { 
        my ($first, $second)=@_;
        ref(first) || ref($second) and die "can't compare non-number\n";
        mal_bool($first>$second);
    },
    '<='=>sub { 
        my ($first, $second)=@_;
        ref(first) || ref($second) and die "can't compare non-number\n";
        mal_bool($first<=$second);
    },
    '>='=>sub { 
        my ($first, $second)=@_;
        ref(first) || ref($second) and die "can't compare non-number\n";
        mal_bool($first>=$second);
    },
);
our @ns=(
    "(def! not (fn* (a) (if a false true)))",
);


1;
