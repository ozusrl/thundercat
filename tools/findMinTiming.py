#!/usr/bin/python
import sys

# Prints the minimum of the command-line arguments

sys.stdout.write(str(min(map(float, sys.argv[1:]))))
 
