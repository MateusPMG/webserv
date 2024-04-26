#!/usr/bin/env python

import sys

def store_data():
    # Read data from standard input
    data = sys.stdin.read().strip()
    
    # Check if data is empty
    if not data:
        print("No data received.")
        return

    # Store data in a file
    with open("var/www/uploads/stored_data.txt", "w") as f:
        f.write(data)

    print("Data stored successfully.")

if __name__ == "__main__":
    store_data()

