# import os
# import sys
# # while True:
# #     a = 'a'

# # print("status: 502\r")
# print("Content-Type: text/html; charset=utf-8\r")
# print("Content-Type: text/html; charset=utf-8\r")

# print("\r")  # End of headers

# print("<!DOCTYPE html>")
# print("<html>")
# print("<head><title>CGI Test</title></head>")
# print("<body>")
# print("<h1>✅ CGI is working!</h1>")
# print("<h2>Environment</h2>")
# print("<pre>")
# for k, v in sorted(os.environ.items()):
#     print(f"{k}={v}\r")
# print("</pre>\r")

# print("<h2>STDIN</h2>")
# # data = sys.stdin.read()
# print("<pre>")
# # print(data if data else "(empty)")
# print("</pre>")

# print("</body>")
# print("</html>")


import sys

def chunk(data):
    sys.stdout.write(f"{len(data.encode()):X}\r\n")
    sys.stdout.write(data)
    sys.stdout.write("\r\n")

# CGI Headers
sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n")
sys.stdout.write("Transfer-Encoding: chunked\r\n")
sys.stdout.write("\r\n")

chunk("<!DOCTYPE html>\n")
chunk("<html>\n")
chunk("<head><title>Chunked CGI Test</title></head>\n")
chunk("<body>\n")
chunk("<h1>Chunked Transfer Encoding Test</h1>\n")
chunk("<p>If you can read this, your server is parsing chunks.</p>\n")

chunk("<h2>STDIN</h2>\r")
print("HDDH\r")
#data = sys.stdin.read()
chunk("<pre>\n")
# chunk(data if data else "(empty)")
chunk("</pre>\n")

# Medium chunk
medium = (
    "<h2>Medium Chunk</h2>\n"
    "<p>"
    + ("ABCDEFGHIJKLMNOPQRSTUVWXYZ " * 10)
    + "</p>\n"
)
chunk(medium)

# Large chunk (~10 KB)
large = (
    "<h2>Large Chunk</h2>\n"
    "<pre>\n"
    + ("0123456789ABCDEF" * 640)
    + "\n</pre>\n"
)
chunk(large)

# Single-character chunks
for c in "CHUNKED":
    chunk(c)

chunk("<hr>\n")

# Numbers
for i in range(100):
    chunk(f"Line {i:03d}: {'*' * (i % 40)}\n")

chunk("<hr>\n")

# HTML table
table = "<table border='1'>\n"
for r in range(15):
    table += "<tr>"
    for c in range(8):
        table += f"<td>{r},{c}</td>"
    table += "</tr>\n"
table += "</table>\n"
chunk(table)

chunk("</body>\n")
chunk("</html>\n")

# Final chunk
sys.stdout.write("0\r\n\r\n")
sys.stdout.flush()

# print("15\r")
# print("<!DOCTYPE html>\r")
# print("6\r")
# print("<html>\r")
# print("36\r")
# print("<head><title>CGI Test</title></head>\r")
# print("6\r")
# print("<body>\r")
# print("43\r")
# print("<h1>:white_check_mark: CGI is working!</h1>\r")
# print("20\r")
# print("<h2>Environment</h2>\r")
# # print("<pre>\r")
# # for k, v in sorted(os.environ.items()):
# #     print(f"{k}={v}")
# # print("</pre>\r")

# # print("<h2>STDIN</h2>")
# # data = sys.stdin.read()
# # print("<pre>")
# # print(data if data else "(empty)")
# # print("</pre>")
# print("7\r")
# print("</body>\r")
# print("7\r")
# print("</html>\r")
# print("0\r")
# print("\r")
