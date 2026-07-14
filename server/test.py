#!/usr/bin/env python3

import os
import sys

sys.stdout.write("Content-Type: text/plain\r\n")
sys.stdout.write("\r\n")

for key, value in sorted(os.environ.items()):
    sys.stdout.write(f"{key}={value}\r\n")