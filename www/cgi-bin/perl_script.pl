#!/usr/bin/env perl
use strict;
use warnings;

# 1. Print required HTTP headers for the webserver
print "Content-Type: text/html\r\n";
print "\r\n";

# 2. Print the HTML body
print "<html>\n";
print "<head><title>CGI Perl Test</title></head>\n";
print "<body>\n";
print "<h1>Hello from Perl CGI!</h1>\n";
print "<p>Your webserv successfully executed this script.</p>\n";
print "</body>\n";
print "</html>\n";
