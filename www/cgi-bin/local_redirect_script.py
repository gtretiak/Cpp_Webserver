#!/usr/bin/env python3

#local redirect rules:
# no status code header
# location header field starts with '/'
# local redirect usually has no body(payload)

import sys

sys.stdout.write("location: /cgi-bin/perl_script.pl?/images/test.png\r\n")

sys.stdout.write("\r\n")

# must internally reprocess the request as if the client requested the new path
# maybe convert this response into HttpRequest and call handle request.


#HANDLE SCRIPT SENDING MALFORMED LOCATION HEADER FIELD