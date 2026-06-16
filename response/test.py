#!/usr/bin/env python3

import sys

# Read body from stdin

# CGI response headers
print("Content-Type: text/html")
print()

# HTML body
print("<html>")
print("<body>")
print("<h1>CGI Script Works!</h1>")
print("<p>Body data:</p>")
print("<pre>%s</pre>" % data)
print("</body>")
print("</html>")