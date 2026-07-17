#!/usr/bin/env python3
import os
import sys


def get_env(*names):
    for n in names:
        if n in os.environ:
            return os.environ[n]
    return None


length_raw = get_env("CONTENT_LENGTH", "CONTENT-LENGTH") or "0"
try:
    length = int(length_raw)
except ValueError:
    length = 0

body = sys.stdin.read(length) if length > 0 else ""

print("Content-Type: text/plain", end="\r\n")
print("", end="\r\n")

print("=== All CGI meta-variables received (raw, as-is) ===")
for key in sorted(os.environ.keys()):
    print("%s = %s" % (key, os.environ[key]))

print()
print("=== Standard CGI/1.1 variable names (underscore, per RFC 3875) ===")
for name in ["REQUEST_METHOD", "SCRIPT_NAME", "PATH_INFO", "PATH_TRANSLATED",
             "QUERY_STRING", "CONTENT_TYPE", "CONTENT_LENGTH", "SERVER_PROTOCOL",
             "GATEWAY_INTERFACE", "SERVER_SOFTWARE"]:
    print("%s = %s" % (name, os.environ.get(name, "(missing)")))

print()
print("=== Request body ===")
print(body if body else "(empty)")
