#!/usr/bin/env python3
# Single Location header, absolute URI, empty body -> CGI_CLIENT_REDIR
print("Location: http://example.com/", end="\r\n")
print("", end="\r\n")
