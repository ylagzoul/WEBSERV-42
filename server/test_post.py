#!/usr/bin/env python3
# Handles HTML form submission

import os
import sys

method = os.environ.get("REQUEST_METHOD", "")
content_length = int(os.environ.get("CONTENT_LENGTH", 0))

# ── Read POST data from stdin ─────────────
post_data = ""
if method == "POST" and content_length > 0:
    post_data = sys.stdin.read(content_length)
    # post_data = "username=john&password=secret"

# ── Parse POST data ───────────────────────
params = {}
for pair in post_data.split("&"):
    if "=" in pair:
        key, value = pair.split("=", 1)
        params[key] = value

username = params.get("username", "unknown")

# ── Output ────────────────────────────────
print("Content-Type: text/html")
print()

print(f"""
<html>
<body>
    <h1>Welcome, {username}!</h1>
</body>
</html>
""")