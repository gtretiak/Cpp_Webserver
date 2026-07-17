#!/usr/bin/env python3
# Status + absolute-URI Location + Content-Type, non-empty body -> CGI_CLIENT_DOC_REDIR
print("Status: 302 Found", end="\r\n")
print("Location: http://example.com/", end="\r\n")
print("Content-Type: text/html", end="\r\n")
print("", end="\r\n")
print("<html><body><p>See other location.</p></body></html>")
