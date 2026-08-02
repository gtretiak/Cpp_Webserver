#!/usr/bin/env python3
# Second hop: redirects again into another CGI script, so a full request
# has to survive two chained CGI_LOCAL_REDIR hops before hitting echo.py.
print("Location: /cgi-bin/echo.py", end="\r\n")
print("", end="\r\n")
