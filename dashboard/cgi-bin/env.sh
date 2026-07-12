#!/bin/bash

# Required HTTP Headers
echo -ne "Content-Type: text/plain\r\n"
echo -ne "Connection: close\r\n\r\n"

echo "=== CGI Environment & STDIN Dump ==="
echo "--- Environment Variables ---"
echo "REQUEST_METHOD: $REQUEST_METHOD"
echo "QUERY_STRING: $QUERY_STRING"
echo "CONTENT_LENGTH: $CONTENT_LENGTH"
echo "CONTENT_TYPE: $CONTENT_TYPE"
echo "HTTP_USER_AGENT: $HTTP_USER_AGENT"
echo ""

echo "--- STDIN (Request Body) ---"
# If it's a POST request and there is a body, read it from standard input
if [ "$REQUEST_METHOD" = "POST" ] && [ -n "$CONTENT_LENGTH" ] && [ "$CONTENT_LENGTH" -gt 0 ]; then
    # Read exactly CONTENT_LENGTH bytes from STDIN
    read -n "$CONTENT_LENGTH" post_body
    echo "Body received: '$post_body'"
else
    echo "No STDIN body provided or not a POST request."
fi