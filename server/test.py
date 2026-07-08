import os
import sys
# while True:
#   a = 'a'

# print("status: 502\r")
print("status: 400\r")
# print("Content-Type: text/plain; charset=utf-8\r")
# print("content-length: 5")

print("Content-Type: text/html; charset=utf-8\r")
# print("content-length: 5\r")
print("\r")


# sys.stdout.write("Transfer-Encoding: chunked\r\n")
# sys.stdout.write("\r\n")

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
# data = sys.stdin.read()
print("<pre>")
# print(data if data else "(empty)")
print("</pre>")

print("</body>")
print("</html>")
