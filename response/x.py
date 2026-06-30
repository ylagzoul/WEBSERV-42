#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html; charset=utf-8\r")
print("transfer-Encoding: chunked\r")
print("\r")  # End of headers

print("15\r")
print("<!DOCTYPE html>\r")
print("6\r")
print("<html>\r")
print("36\r")
print("<head><title>CGI Test</title></head>\r")
print("6\r")
print("<body>\r")
print("43\r")
print("<h1>:white_check_mark: CGI is working!</h1>\r")
print("20\r")
print("<h2>Environment</h2>\r")
# print("<pre>\r")
# for k, v in sorted(os.environ.items()):
#     print(f"{k}={v}")
# print("</pre>\r")

# print("<h2>STDIN</h2>")
# data = sys.stdin.read()
# print("<pre>")
# print(data if data else "(empty)")
# print("</pre>")
print("7\r")
print("</body>\r")
print("7\r")
print("</html>\r")
print("0\r")
print("\r")