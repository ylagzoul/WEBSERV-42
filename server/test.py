#!/usr/bin/env python3
import os
import sys

# Define the CRLF delimiter
CRLF = b"\r\n"

# 1. Prepare the HTTP Headers
headers = [
    b"Content-Type: text/html; charset=utf-8",
    b"Content-length: 400",
    b"Cache-Control: no-cache",

]

# 2. Prepare the HTML Body
html_body = """<!DOCTYPE html>
<html>
<head>
    <title>CGI Test Success</title>
</head>
<body>
    <h1>Hello from Python CGI!</h1>
    <p>This page was generated without using the old <code>cgi</code> library.</p>
    <h2>Environment Details:</h2>
    <ul>
        <li><strong>Gateway Interface:</strong> {gateway}</li>
        <li><strong>Request Method:</strong> {method}</li>
        <li><strong>Query String:</strong> {query}</li>
    </ul>
</body>
</html>
""".format(
    gateway=os.environ.get('GATEWAY_INTERFACE', 'Not found'),
    method=os.environ.get('REQUEST_METHOD', 'Not found'),
    query=os.environ.get('QUERY_STRING', 'None')
)

# 3. Write everything to stdout as bytes
try:
    # Write headers joined by CRLF
    for header in headers:
        sys.stdout.buffer.write(header + CRLF)
    
    # Crucial: An empty line (extra CRLF) signals the end of headers
    sys.stdout.buffer.write(CRLF)
    
    # Write the body encoded as bytes
    sys.stdout.buffer.write(html_body.encode('utf-8'))
    sys.stdout.flush()
except Exception as e:
    # Fallback minimal error handling
    sys.stderr.write(f"Error in CGI script: {str(e)}\n")