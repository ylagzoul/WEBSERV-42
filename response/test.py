#!/usr/bin/env python3

import os
import sys

print("status: 302\r")
print("Content-Type: text/plain; charset=utf-8\r")
print("location: /server\ncontent-length: 5\r")
print("\r")