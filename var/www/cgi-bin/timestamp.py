#!/usr/bin/env python3

import cgi
import cgitb
import datetime

# Enable CGI error reporting
cgitb.enable()


# Get the current timestamp
timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Print the timestamp as the response
print(timestamp)
