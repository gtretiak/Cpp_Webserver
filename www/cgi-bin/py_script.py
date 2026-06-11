#!/usr/bin/env python3
import sys

#IMPORTANT Note: the print() function appends a newline '\n' to the output.

sys.stdout.write("coNteNt-TYPe: text/html\r\n")

sys.stdout.write("Status: 200 OK\r\n")
sys.stdout.write("Set-Cookie: session=abc123; Path=/; HttpOnly\r\n")
sys.stdout.write("Set-Cookie: theme=dark; Path=/\r\n")
sys.stdout.write("Cache-Control: no-cache, no-store, must-revalidate\r\n")
sys.stdout.write("Pragma: no-cache\r\n")
sys.stdout.write("Expires: 0\r\n")
#sys.stdout.write("location: /cgi-bin/perl_script.py\r\n")
sys.stdout.write("Content-Language: en-US\r\n")
sys.stdout.write("stAtus: 300 OK\r\n")

sys.stdout.write("\r\n")

# 2. Print the HTML body
sys.stdout.write("<html><body>\n")
sys.stdout.write("<h1>CGI Test</h1>\n")
sys.stdout.write("<p>This is a test CGI response.</p>\n")
sys.stdout.write("</body></html>\n")
