package Core;
use strict;
use warnings;
use Scalar::Util;
use Time::HiRes qw(time);
use Reader;
use Printer;

our %metadata;

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
    '*host-language*'=>do { my $host='perl'; bless \$host, "MalString"; },
    '+'=>sub { my $val=0; $val+=shift while @_; return $val; },
    '-'=>sub { my $val=shift; $val-=shift while @_; return $val; },
    '*'=>sub { my $val=1; $val*=shift while @_; return $val; },
    '/'=>sub { my $val=shift; $val=int($val/shift) while @_; return $val; },
    'pr-str'=>sub {
        my $str=join ' ', map { Printer::pr_str($_) } @_;
        return bless \$str, 'MalString';
    },
    str=>sub {
        my $str=join '', map { Printer::pr_str($_, 0) } @_;
        return bless \$str, 'MalString';
    },
    prn=>sub {
        my @str=map { Printer::pr_str($_) } @_;
        print join(" ", @str), "\n";
        return 'nil'
    },
    println=>sub {
        my @str=map { Printer::pr_str($_, 0) } @_;
        print join(" ", @str), "\n";
        return 'nil'
    },
    list=>sub { bless [@_], 'MalList' },
    'list?'=>sub { mal_bool(ref(shift) eq 'MalList'); },
    'atom?'=>sub { mal_bool(ref(shift) eq 'MalAtom'); },
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
        ref($first) || ref($second) and die "can't compare non-number\n";
        mal_bool($first<$second);
    },
    '>'=>sub { 
        my ($first, $second)=@_;
        ref($first) || ref($second) and die "can't compare non-number\n";
        mal_bool($first>$second);
    },
    '<='=>sub { 
        my ($first, $second)=@_;
        ref($first) || ref($second) and die "can't compare non-number\n";
        mal_bool($first<=$second);
    },
    '>='=>sub { 
        my ($first, $second)=@_;
        ref($first) || ref($second) and die "can't compare non-number\n";
        mal_bool($first>=$second);
    },
    'read-string'=>sub {
        my $str=shift;
        ref($str) eq 'MalString' or die "can't read from non-string\n";
        return Reader::read_str($$str)//'nil';
    },
    slurp=>sub {
        my $fn=shift;
        ref($fn) eq 'MalString' or die "can't open non-string\n";
        open(my $fh, '<', $$fn) or die "can't open $$fn\n";
        my $str=do { local $/; <$fh>; };
        return bless \$str, 'MalString';
    },
    eval=>sub { @_=shift; goto &::EVAL; },
    atom=>sub { my $value=shift; return bless \$value, 'MalAtom'; },
    deref=>sub {
        my $atom=shift;
        return ref $atom eq 'MalAtom' ? $$atom : 'nil';
    },
    'reset!'=>sub {
        my ($atom, $value)=@_;
        $$atom=$value if ref $atom eq 'MalAtom';
        return $value;
    },
    'swap!'=>sub {
        my ($atom, $fn, @args)=@_;
        die "bad function\n" unless ref $fn eq 'CODE';
        my $value=ref $atom eq 'MalAtom' ? $$atom : 'nil';
        $value=$fn->($value, @args);
        $$atom=$value if ref $atom eq 'MalAtom';
        return $value;
    },
    cons=>sub {
        my ($atom, $list)=@_;
        die "bad argument to cons\n" unless ref $list eq 'MalList' || ref $list eq 'MalVector' || $list eq 'nil';
        return bless [ $atom, ref $list ? @$list : () ], 'MalList';
    },
    concat=>sub {
        my $list=bless [], 'MalList';
        for(@_) {
            die "bad argument to concat\n" unless ref eq 'MalList' || ref eq 'MalVector' || $_ eq 'nil';
            push @$list, @$_ if ref;
        }
        return $list;
    },
    vec=>sub {
        my $vec=shift;
        return $vec unless ref $vec eq 'MalList';
        return bless [ @$vec ], 'MalVector';
    },
    nth=>sub {
        my ($list, $index)=@_;
        ref $list eq 'MalList' || ref $list eq 'MalVector' or die "bad list in nth\n"; 
        Scalar::Util::looks_like_number($index) && $index>=0 && $index <@$list or die "index out of bounds\n";
        return $list->[$index];
    },
    first=>sub {
        my $list=shift;
        return 'nil' if ref $list ne 'MalList' and ref $list ne 'MalVector';
        return 'nil' unless @$list;
        return $list->[0];
    },
    rest=>sub {
        my $list=shift;
        my @rest;
        (undef, @rest)=@$list if ref $list eq 'MalList' or ref $list eq 'MalVector';
        return bless \@rest, 'MalList';
    },
    throw=>sub {
        my $exception=shift;
        $exception="$exception\n" unless ref $exception;
        die $exception;
    },
    apply=>sub {
        my $fn=shift;
        ref $fn eq 'CODE' or die "bad apply\n";
        if(@_) {
            my $list=pop;
            ref $list eq 'MalList' or ref $list eq 'MalVector' or die "bad apply\n";
            push @_, @$list;
        }
        goto &$fn;
    },
    map=>sub {
        my ($fn, $list)=@_;
        ref $fn eq 'CODE' and (ref $list eq 'MalList' or ref $list eq 'MalVector') or die "bad map\n";
        return bless [ map { $fn->($_) } @$list ], 'MalList';
    },
    'nil?'=>sub { mal_bool(shift eq 'nil') },
    'true?'=>sub { mal_bool(shift eq 'true') },
    'false?'=>sub { mal_bool(shift eq 'false') },
    'symbol?'=>sub { mal_bool(ref shift eq 'MalSymbol') },
    'vector?'=>sub { mal_bool(ref shift eq 'MalVector') },
    'keyword?'=>sub { mal_bool(ref shift eq 'MalKeyword') },
    'string?'=>sub { mal_bool(ref shift eq 'MalString') },
    'number?'=>sub { my $val=shift; mal_bool(!ref $val && Scalar::Util::looks_like_number($val)) },
    'fn?'=>sub { mal_bool(ref shift eq 'CODE') },
    'macro?'=>sub { mal_bool(ref shift eq 'MalMacro') },
    'sequential?'=>sub { my $val=shift; mal_bool(ref $val eq 'MalList' or ref $val eq 'MalVector') },
    'map?'=>sub { mal_bool(ref shift eq 'MalHash') },
    symbol=>sub {
        my $symbol=shift;
        ref $symbol eq 'MalString' or die "Bad symbol\n";
        my $name=$$symbol;
        return bless \$name, 'MalSymbol';
    },
    keyword=>sub {
        my $symbol=shift;
        return $symbol if ref $symbol eq 'MalKeyword';
        ref $symbol eq 'MalString' or die "Bad keyword\n";
        my $name="\0:$$symbol";
        return bless \$name, 'MalKeyword';
    },
    vector=>sub { bless [@_], 'MalVector' },
    'hash-map'=>sub {
        @_ % 2 and die "bad hash-map\n";
        my $hash=bless { }, 'MalHash';
        while(@_) {
            my $key=Reader::freeze_key(shift);
            $hash->{$key}=shift;
        }
        return $hash;
    },
    assoc=>sub {
        my $old_hash=shift;
        ref $old_hash ne 'MalHash' || @_ % 2 and die "bad assoc\n";
        my $hash = bless { %$old_hash }, 'MalHash';
        while(@_) {
            my $key=Reader::freeze_key(shift);
            $hash->{$key}=shift;
        }
        return $hash;
    },
    dissoc=>sub {
        my $old_hash=shift;
        ref $old_hash ne 'MalHash' and die "bad dissoc\n";
        my $new_hash=bless { %$old_hash }, 'MalHash';
        delete $new_hash->{Reader::freeze_key($_)} for @_;
        return $new_hash;
    },
    get=>sub {
        my ($hash, $key)=@_;
        return 'nil' if $hash eq 'nil';
        ref $hash eq 'MalHash' or die "bad get\n";
        return $hash->{Reader::freeze_key($key)} // 'nil';
    },
    'contains?'=>sub {
        my ($hash, $key)=@_;
        return 'false' if $hash eq 'nil';
        ref $hash eq 'MalHash' or die "bad get\n";
        ref $hash eq 'MalHash' or die "bad contains?\n";
        return mal_bool(exists $hash->{Reader::freeze_key($key)});
    },
    keys=>sub {
        my $hash=shift;
        ref $hash eq 'MalHash' or die "bad keys\n";
        return bless [ map { Printer::thaw_key($_) } keys %$hash ], 'MalList';
    },
    vals=>sub {
        my $hash=shift;
        ref $hash eq 'MalHash' or die "bad vals\n";
        return bless [ values %$hash ], 'MalList';
    },
    'time-ms'=>sub { int(time()*1000) },
    seq=>sub {
        my $seq=shift;
        if($seq eq 'nil') {
            return 'nil';
        } elsif(ref $seq eq 'MalList') {
            return @$seq ? $seq : 'nil';
        } elsif(ref $seq eq 'MalVector') {
            return @$seq ? bless [@$seq], 'MalList' : 'nil';
        } elsif(ref $seq eq 'MalString') {
            my @string=map { my $ch=$_; bless \$ch, 'MalString' }  split //, $$seq;
            return @string ? bless [@string], 'MalList' : 'nil';
        }
        die "bad seq\n";
    },
    conj=>sub {
        my $collection=shift;
        ref $collection eq 'MalList' and return bless [reverse(@_), @$collection], 'MalList';
        ref $collection eq 'MalVector' and return bless [@$collection, @_], 'MalVector';
        die "bad conj\n"; 
    },
    meta=>sub {
        my $val=shift;
        my $addr=Scalar::Util::refaddr($val) // 'undefined';
        return $metadata{$addr}//'nil';
    },
    'with-meta'=>sub {
        my ($val, $meta)=@_;
        my $data=$val;
        if(ref $val eq 'MalVector' or ref $val eq 'MalList') {
            $data=bless [@$val], ref $val;
            $metadata{Scalar::Util::refaddr($data)}=$meta;
        } elsif(ref $data eq 'MalHash') {
            $data=bless { %$val }, 'MalHash';
            $metadata{Scalar::Util::refaddr($data)}=$meta;
        } elsif(ref $val eq 'CODE') {
            $data=sub { goto &$val };
            $metadata{Scalar::Util::refaddr($data)}=$meta;
        }
        return $data;
    },
    readline=>sub {
        my $prompt=shift;
        $prompt=(ref $prompt eq 'MalString') ? $$prompt : '> ';
        my $line=$::term->readline($prompt);
        return defined $line ? bless \$line, 'MalString' : 'nil';
    }
);


1;
