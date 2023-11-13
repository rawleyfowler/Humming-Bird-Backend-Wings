no precompilation;

use NativeCall;
use nqp;
use Humming-Bird::Glue;
use Humming-Bird::Backend;

constant $LIB ='resources/libraries/wings';

unit class Humming-Bird::Backend::Wings does Humming-Bird::Backend;

my sub blob-from-pointer(Pointer:D \ptr, Int :$elems!, Blob:U :$type = Buf) is export {
    my sub memcpy(Blob:D $dest, Pointer $src, size_t $size) returns Pointer is native(Str) { * };
    my \t = ptr.of ~~ void ?? $type.of !! ptr.of;
    if  nativesizeof(t) != nativesizeof($type.of) {
        fail "Pointer type don't match Blob type";
    }
    my $b = $type;
    with ptr {
        if $b.can('allocate') {
            $b .= allocate($elems);
        } else {
            $b = nqp::setelems($b, nqp::unbox_i($elems.Int));
        }
        memcpy($b, ptr, $elems * nativesizeof(t));
    }
    $b;
}
sub WingsListen(int32 $port,
                Str $addr,
                &callback (Pointer[int8], int32, int32 --> Pointer[int8]),
                &size_callback (int32 --> uint64)) is nativeconv('thisgnu') is native($LIB) { * }


has %!connections;
has &!handler;

method size_handler {
    return sub (int32 $pid) {
        return %!connections{$pid}<response>:exists ?? %!connections{$pid}<response>.bytes !! 0;

        END {
            %!connections{$pid}:delete;
        }
    }
}

method request_handler {
    return sub (Pointer[int8] $raw-request, int32 $size, int32 $pid) {
        CATCH { default { .say; return Nil } };
        my Buf $buf = blob-from-pointer(
            $raw-request,
            elems => $size,
            type => Buf[int8]
        );

        my $request;
        my $header-req = False;
        if %!connections{$pid}<request>:exists {
            $request = %!connections{$pid};
        }
        else {
            $header-req = True;
            $request = %!connections{$pid}<request> = Humming-Bird::Glue::Request.decode($buf);
        }

        $request.body.append: $buf unless $header-req;

        my $response;
        with $request.header('Content-Length') {
            if ($request.body.bytes == $request.header('Content-Length')) {
                $response = %!connections{$pid}<response> = &!handler($request).encode($request.method !== HEAD);
            }
            else {
                return Nil; # Tell the server we need more data before we can complete the connection.
            }
        }

        $response //= %!connections{$pid}<response> = &!handler($request).encode($request.method !== HEAD);

        return nativecast(Pointer[int8], $response); # Include body if not a HEAD request.
    };
}

method listen(&handler) {
    &!handler := &handler;
    WingsListen(8080, '0.0.0.0', self.request_handler, self.size_handler);
}
