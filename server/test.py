import os
import sys

print("status: 400\r")

print("Content-Type: text/html; charset=utf-8\r")
print("\r")


print("<!DOCTYPE html>")
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>✅ CGI is working!</h1>")
print("<h2>Environment</h2>")
print("<pre>")
for k, v in sorted(os.environ.items()):
    print(f"{k}={v}\r")
print("</pre>\r")

print("<h2>STDIN</h2>")
print("<pre>")
print("</pre>")

print("</body>")
print("</html>")