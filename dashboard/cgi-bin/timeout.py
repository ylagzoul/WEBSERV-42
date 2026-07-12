#!/usr/bin/env python3
import time
import sys

# Flush output to ensure headers are sent immediately (if testing chunked timeout)
print("Content-Type: text/html\r\n\r\n", end="")
sys.stdout.flush()

# Infinite loop to test if the C++ server's epoll/poll catches the CGI hanging
while True:
    time.sleep(1)