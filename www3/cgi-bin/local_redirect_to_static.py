#!/usr/bin/env python3
# Single Location header, abs-path, empty body -> CGI_LOCAL_REDIR
# Target is NOT under /cgi-bin, so Router should resolve it as a static file.
print("Location: /upload.html", end="\r\n")
print("", end="\r\n")
