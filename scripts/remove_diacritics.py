#!/usr/bin/env python

import unicodedata
import sys
import re

"""
Script for removing diacritics in the data. Requires utf-8 input.

Usage:
------
cat file.txt | python remove_diacritics.py

"""

diacritics = ur'[\u0301\u030a\u030c]'

for line in sys.stdin:
    print line
    line_uni = unicode(line, "utf-8")
    normalized_line = unicodedata.normalize('NFD', line_uni)
    print re.sub(diacritics, '', normalized_line)


