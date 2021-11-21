package Env;

our $parent="\0";

sub new {
    my $self=shift;
    my $class=ref($self)||$self;
    my $obj=bless {}, $class;
    $obj->set(@_) if @_;
    return $obj;
}

sub extend {
    my $self=shift;
    my $obj=bless { $parent=>$self }, ref $self;
    $obj->set(@_) if @_;
    return $obj;
}

sub set {
    my $self=shift;
    if(@_==1) {
        my $binding=shift;
        @_=@$binding if ref $binding eq 'MalList' || ref $binding eq 'MalVector';
    }
    @_>0 && @_%2==0 or die "malformed binding\n"; 
    while(@_) {
        my $key=shift;
        if(ref $key) {
            ref $key eq 'MalSymbol' or die "bad name type\n";
            $key=$$key;
        }
        $self->{$key}=shift;
    }
    return $self;
}

sub find {
    my ($self, $key)=@_;
    exists $self->{$key} and return $self;
    exists $self->{$parent} and return $self->{$parent}->find($key);
    return;
}

sub get {
    my ($self, $key)=@_;
    my $env=$self->find($key);
    return $env->{$key} if $env;
    die "$key not found\n";
}

1;
