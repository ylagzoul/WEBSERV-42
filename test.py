#!/usr/bin/env python3

import os
import sys

print("status: 200\r")
print("content-type: text/html\r")
print("\r")

print("<html>")
print("<body>")
print("<h1>CGI IS WORKING</h1>")
print("<pre>\r")
for k, v in sorted(os.environ.items()):
    print(f"{k}={v}")
print("</pre>\r")
print("</body>")
print("</html>")