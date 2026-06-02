#!/usr/bin/env python3
# file: /var/www/cgi-bin/hello.py

import os

# ── Read environment variables ────────────
method       = os.environ.get("REQUEST_METHOD", "")
query_string = os.environ.get("QUERY_STRING",   "")
remote_addr  = os.environ.get("REMOTE_ADDR",    "")

# ── Parse query string ────────────────────
# query_string = "name=John&age=25"
params = {}
if query_string:
    for pair in query_string.split("&"):
        if "=" in pair:
            key, value = pair.split("=", 1)
            params[key] = value

name = params.get("name", "World")

# ── Output HTTP headers FIRST ─────────────
print("Content-Type: text/html")
print()    # blank line — REQUIRED!

# ── Output HTML body ──────────────────────
print(f"""
<html>
<head><title>CGI Hello</title></head>
<body>
    <h1>Hello, {name}!</h1>
    <p>Your IP: {remote_addr}</p>
    <p>Method: {method}</p>
</body>
</html>
""")