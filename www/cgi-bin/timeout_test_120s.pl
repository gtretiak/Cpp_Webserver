#!/usr/bin/env perl
use strict;
use warnings;

sleep(120);

print "Status: 200 OK\r\n";
print "Content-Type: text/plain\r\n\r\n";
print "Perl CGI finished after 2 minutes.\n";