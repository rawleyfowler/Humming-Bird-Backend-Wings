no precompilation;

use NativeCall;
use Humming-Bird::Glue;
use Humming-Bird::Backend;

constant $LIB ='resources/libraries/wings';

unit class Humming-Bird::Backend::Wings does Humming-Bird::Backend;

sub WingsListen(int32 $port, Str $addr, &callback (Pointer[int8], int32 --> Pointer[int8])) is nativeconv('thisgnu') is native($LIB) { * }

has %!connections;
has &!handler;

method request_handler(CArray[int8] $raw-request, int32 $pid) {
   
    my $buf = Buf.new($raw-request.Slip);

    my $request;
    my $header-req = False;
    if %!connections{$pid}:exists {
        $request = %!connections{$pid};
    }
    else {
        $header-req = True;
        $request = %!connections{$pid} = Humming-Bird::Glue::Request.decode($buf);
    }

    $request.body.append: $buf unless $header-req;

    my $response;
    with $request.header('Content-Length') {
        if ($request.body.bytes == $request.header('Content-Length')) {
            $response = &!handler($request);
        }
        else {
            return Nil; # Tell the server we need more data before we can complete the connection.
        }
    }

    $response //= &!handler($request);

    return CArray[int8].new: $response.encode($request.method !== HEAD); # Include body if not a HEAD request.
}

method listen(&handler) {
    &!handler := &handler;
    WingsListen(8080, '0.0.0.0', self.^lookup('request_handler').assuming(self));
}
