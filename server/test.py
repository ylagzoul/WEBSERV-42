#!/usr/bin/env python3

import os
import sys
# while True:
#     a = 'a'
print("Content-Type: text/html; charset=utf-8\r")
print("status: 400\r")
print("\r")  # End of headers

print("<!DOCTYPE html>\r")
print("<html>\r")
print("<head><title>CGI Test</title></head>\r")
print("<body>\r")
print("<h1>✅ CGI is working!</h1>\r")

print("<h2>Environment</h2>\r")
print("<pre>\r")
for k, v in sorted(os.environ.items()):
    print(f"{k}={v}\r")
print("</pre>\r")

# print("<h2>STDIN</h2>")
# data = sys.stdin.read()
# print("<pre>")
# print(data if data else "(empty)")
# print("</pre>")

# print("</body>")
# print("</html>")
