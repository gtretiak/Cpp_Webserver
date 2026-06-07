#!/usr/bin/env python3
import sys

# 1. Print required HTTP headers for the webserver
print("Content-Type: text/html\r")
print("\r")

# 2. Print the HTML body
print("<html>")
print("<head><title>CGI Python Test</title></head>")
print("<body>")
print("<h1>Hello from Python CGI!</h1>")
print("<p>Your webserv successfully executed this script.</p>")
print("</body>")
print("</html>")
