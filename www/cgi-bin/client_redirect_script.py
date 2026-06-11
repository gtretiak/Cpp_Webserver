#!/usr/bin/env python3

import sys

#client redirect rules:
# ONLY location header field containing a absolut URL
	#example: https://google.com/
# no status code header
# no content-type

sys.stdout.write("location: https://www.google.com\r\n")

sys.stdout.write("\r\n")


# server must send a 302 response
