#!/bin/bash

# Checks for compliance with 
# Rule: 'Rules on if, for, while, etc. imply that there is no '){' allowed.'
#

# Steps:
# - exclude src/docs/ holding the documentation only, and ftn-auto directories
# - get all lines with '){'


find src/ -name *.[ch] -or -name *.cu \
 | grep -v 'src/docs' \
 | grep -v 'ftn-auto' \
 | xargs grep "){"
