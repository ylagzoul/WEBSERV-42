#!/usr/bin/env python3
import os
import uuid

# Read the HTTP_COOKIE environment variable set by your C++ server
cookie_header = os.environ.get('HTTP_COOKIE', '')
session_id = None

# Parse the cookie safely
if 'session_id=' in cookie_header:
    parts = cookie_header.split('session_id=')
    if len(parts) > 1:
        session_id = parts[1].split(';')[0]

print("Content-Type: text/plain\r")

if not session_id:
    # Set a new cookie via headers
    session_id = str(uuid.uuid4())
    print(f"Set-Cookie: session_id={session_id}; Path=/; HttpOnly\r")
    print("\r\n=== Session Created ===\n", end="")
    print(f"New Session ID: {session_id}")
else:
    print("\r\n=== Session Found ===\n", end="")
    print(f"Welcome back! Session ID: {session_id}")

print(f"Raw HTTP_COOKIE received: '{cookie_header}'")