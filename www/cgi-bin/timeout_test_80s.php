#!/usr/bin/env php
<?php
sleep(80);

echo "Status: 200 OK\r\n";
echo "Content-Type: text/plain\r\n\r\n";
echo "PHP CGI finished after 80 seconds.\n";
?>