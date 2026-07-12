#!/usr/bin/env python3
import sys
import os

# 1. Print mandatory CGI headers (CRLF is required by HTTP)
print("Content-Type: text/plain\r\n\r\n", end="")

print("=== Python CGI STDIN Dump ===")

# 2. Grab environment variables passed by your C++ server via execve()
method = os.environ.get("REQUEST_METHOD", "UNKNOWN")
content_length_str = os.environ.get("CONTENT_LENGTH", "0")
content_type = os.environ.get("CONTENT_TYPE", "UNKNOWN")

print(f"Method: {method}")
print(f"Content-Length: {content_length_str}")
print(f"Content-Type: {content_type}")
print("-" * 30)

# 3. Read STDIN if it's a POST request with a body
if method == "POST" and content_length_str.isdigit():
    content_length = int(content_length_str)
    
    if content_length > 0:
        # CRITICAL: Read exactly 'content_length' bytes. 
        # Do NOT use sys.stdin.read() empty, or the script will hang waiting for EOF
        # if your C++ epoll/select loop keeps the pipe open!
        body = sys.stdin.read(content_length)
        
        print("--- STDIN (Request Body) ---")
        print(body)
    else:
        print("No body content (Content-Length is 0).")
else:
    print("Not a POST request, or no Content-Length provided.")