use NativeCall;
use Humming-Bird::Glue;

constant $LIB = %?RESOURCES<libraries/wings> // 'resources/libraries/libwings.so';

sub wings_listen(Int:D $port, Str:D $addr, &callback) returns Void is native($LIB);

unit class Humming-Bird::Backend::Wings does Humming-Bird::Backend;

has Int:D $.port = 8080;
has Int:D $.timeout = 1;

has %.connections;

method !make-handler(&handler) {
    return sub (CArray[int8]:D $raw-request, Int:D $pid) {
        my $buf = Buf.new($raw-request.Slip);

        my $request;
        if %!connections{$pid}:exists {
            $request = %!connections{$pid};
        }
        else {
            $request = %!connections{$pid} = Humming-Bird::Glue::Request.decode($buf);
        }

        with $request.header('Content-Length') {
            if ($request.body.bytes == $request.header('Content-Length')) {
                return &handler($request).encode;
            }
            else {
                return Nil; # Tell the server we need more data before we can complete the connection.
            }
        }

        return &handler($request).encode; 
    }
}

method listen(&handler) {
    wings_listen(8080, '0.0.0.0', !make-handler(&handler));
}
