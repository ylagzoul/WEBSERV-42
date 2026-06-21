#!/usr/bin/env python3

import sys

image_path = "gogo.jpg"

sys.stdout.buffer.write(b"status: 599 \r\n")
sys.stdout.buffer.write(b"content-length: 300000 \r\n")
sys.stdout.buffer.write(b"content-type: image/jpg\r\n")
sys.stdout.buffer.write(b"\r\n")

with open(image_path, "rb") as f:
    sys.stdout.buffer.write(f.read())