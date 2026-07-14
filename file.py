#!/usr/bin/env python3

import os
import sys

sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("\r\n")

print("<html>")
print("<body>")
print("<pre>")
for key, value in sorted(os.environ.items()):
    sys.stdout.write(f"{key}={value}\r\n")
print("</pre>")
print("</body>")
print("</html>")