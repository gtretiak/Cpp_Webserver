#!/usr/bin/env python3
# Local redirect whose target is itself another CGI script.
# This exercises the CGI_LOCAL_REDIR -> CGI branch in finalizeCgi().
print("Location: /cgi-bin/local_redirect_chain2.py", end="\r\n")
print("", end="\r\n")
