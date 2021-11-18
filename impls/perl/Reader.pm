package Reader;
use Types;
use Scalar::Util;

sub next_token {
    my $self=shift;
    return shift @$self
}
sub peek_token {
    my $self=shift;
    return $self->[0];
}

sub read_str {
    my $str=shift;
    my $reader=tokenize($str);
    return read_form($reader);
}

sub tokenize {
    my $str=shift;
    my $token_re=qr!
            [\s,]*(                  #skip whitespace and commas
            ~@ | [\[\]{}()'`~^@]     #and capture special character   
            | "(?:\\.|[^\\"])*"?     #or quoted string w/ backslash escapes
            | ;.*                    #or semicolon-marked comment
            | [^\s\[\]{}('"`,;)]+    #or identifier
        )!x;
    return bless [$str =~ /$token_re/g], 'Reader'; 
}

my %delim=(
  '('=>')',
  '{'=>'}',
  '['=>']',
  ')'=>undef,
  ']'=>undef,
  '}'=>undef,
);
my %delim_class=(
  '('=>'MalList',
  '['=>'MalVector',
  '{'=>'MalHash',
);
my %quote=(
  "'"=>'quote',
  '`'=>'quasiquote',
  '~'=>'unquote',
  '~@'=>'splice-unquote',
  '@'=>'deref',
);
sub read_form {
    my $reader=shift;
    my $token=$reader->next_token();
    $token or return;
    $token =~ /^;/ and return read_form($reader);
    exists $delim{$token} and return read_list($reader, $token);
    exists $quote{$token} and return read_quote($reader, $token);
    $token eq '^' and return read_meta($reader);
    return read_atom($token);
}

sub read_list {
    my ($reader, $start)=@_;
    die "unbalanced" unless $delim{$start};
    my @list;
    while($reader->peek_token() ne $delim{$start}) {
        my $val=read_form($reader);
        defined $val or die "unbalanced";
        push @list, $val;
    }
    $reader->next_token(); #swallow end delim
    $start ne '{' and return bless [ @list ], $delim_class{$start};

    #make hash
    my $hash=bless {}, $delim_class{$start};

    while(@list) {
        my $key=Types::freeze_key(shift @list);
        @list && $key or die "bad hash";
        $hash->{$key}=shift @list;
    }
    return $hash;
}

sub read_quote {
    my ($reader, $quote)=@_;
    my $name=$quote{$quote};
    return bless [
        Types::intern_symbol($quote{$quote}),
        read_form($reader)
    ], 'MalList';
}

sub read_meta {
    my $reader=shift;
    my $meta=read_form($reader);
    my $data=read_form($reader);
    return bless [
        Types::intern_symbol('with-meta'),
        $data, $meta
    ], 'MalList';
}

sub read_atom {
    my $token=shift;
    Scalar::Util::looks_like_number($token) and return $token;
    $token =~ /"(?:\\.|[^\\"])*"/ and return bless \$token, 'MalString'; 
    $token =~ /^"/ and die "unbalanced";
    return Types::intern_symbol($token);
}


1;
