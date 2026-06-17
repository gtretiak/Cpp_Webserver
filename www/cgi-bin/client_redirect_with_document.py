#!/usr/bin/env python3

import sys

#header section
sys.stdout.write("location: https://www.google.com\r\n")
sys.stdout.write("content-type: text/html\r\n")
sys.stdout.write("status: 302\r\n")

#end of header section
sys.stdout.write("\r\n") 

#body
sys.stdout.write("<html><body>\n")
sys.stdout.write("<h1>CGI Test</h1>\n")
sys.stdout.write("<p>This is a test CGI Redirect Response With Document .</p>\n")
sys.stdout.write("<p>It MUST contain a body, a location header field and a status code .</p>\n")
sys.stdout.write("</body></html>\n")