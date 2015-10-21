#!/usr/bin/python
import sys

# Prints the minimum of the command-line arguments
values = map(float, sys.argv[1:])
sys.stdout.write(str(1+values.index(min(values))))
 
