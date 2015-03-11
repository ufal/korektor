#!/usr/bin/env
import argparse
import codecs
import re
import sys

import spelleval


if __name__ == '__main__':

    # parser the command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("test_file", help="Test file in UTF-8")
    parser.add_argument("gold_file", help="Gold file in UTF-8")
    parser.add_argument("system_output", help="System output (Korektor)")
    args = parser.parse_args()

    korektor_evaluation = spelleval.SpellEval(args.test_file, args.gold_file, args.system_output, 5)
    korektor_evaluation.evaluate()
    korektor_evaluation.print_results()


