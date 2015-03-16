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
    line_uni = unicode(line, "utf-8")
    normalized_line = unicodedata.normalize('NFD', line_uni)
    no_diacritics_line = re.sub(diacritics, '', normalized_line)
    line_utf8 = no_diacritics_line.encode("utf-8")
    print line_utf8


