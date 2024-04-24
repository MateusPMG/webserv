#!/usr/bin/env python3

import cgi
import cgitb
import datetime

# Enable CGI error reporting
cgitb.enable()

# Print necessary headers
print("Content-Type: text/plain")
print()

# Get the current timestamp
timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Print the timestamp as the response
print(timestamp)
