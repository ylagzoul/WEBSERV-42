#!/usr/bin/perl
use strict;
use warnings;

# Ensure autoflush so the C++ server gets headers immediately
$| = 1;

print "Content-Type: text/plain\r\n\r\n";
print "=== CGI Raw Multipart Parser ===\n";

my $content_length = $ENV{'CONTENT_LENGTH'} || 0;
my $content_type = $ENV{'CONTENT_TYPE'} || '';

print "Method: $ENV{'REQUEST_METHOD'}\n";
print "Length: $content_length\n";
print "Type: $content_type\n\n";

if ($ENV{'REQUEST_METHOD'} eq 'POST' && $content_length > 0) {
    # CRITICAL: Prevent Perl from messing with binary upload bytes
    binmode(STDIN);

    my $buffer = "";
    my $total_read = 0;
    my $bytes_read;
    my $chunk;

    # Robust Read Loop: Wait for all bytes from the C++ pipe
    while ($total_read < $content_length) {
        $bytes_read = read(STDIN, $chunk, $content_length - $total_read);
        
        if (!defined $bytes_read) {
            print "ERROR: Read failure at $total_read bytes ($!).\n";
            last;
        }
        if ($bytes_read == 0) {
            print "ERROR: Unexpected EOF. The C++ server closed the pipe early.\n";
            last;
        }
        
        $buffer .= $chunk;
        $total_read += $bytes_read;
    }

    if ($total_read == $content_length) {
        print "SUCCESS: Read exactly $total_read bytes from STDIN.\n\n";
        print "Raw Body Snippet (First 150 chars):\n";
    } else {
        print "FAILED: Expected $content_length bytes, but only got $total_read.\n";
        print "Check your C++ waitpid() or epoll() loop. You aren't writing everything to the pipe.\n";
    }
} else {
    print "No POST data expected, or CONTENT_LENGTH is missing/zero.\n";
}