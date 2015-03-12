#!/usr/bin/env
import argparse
import codecs
import re
import sys

"""
************************************
Spell checker main evaluation script
************************************

How to run:

 python eval_spell_checking.py --help
usage: eval_spell_checking.py [-h] test_file gold_file system_output

positional arguments:
  test_file      Test file in UTF-8
  gold_file      Gold file in UTF-8
  system_output  System output (Korektor)

optional arguments:
  -h, --help     show this help message and exit

"""

import spelleval


if __name__ == '__main__':

    # parser the command line arguments
    parser = argparse.ArgumentParser(description="Spellchecker Evaluation")
    parser.add_argument("test_file", help="Test file in UTF-8")
    parser.add_argument("gold_file", help="Gold file in UTF-8")
    parser.add_argument("system_output", help="System output (Korektor)")
    parser.add_argument("n_best", help="Evaluate top-n suggestions", type=int)

    args = parser.parse_args()

    korektor_evaluation = spelleval.SpellEval(args.test_file, args.gold_file, args.system_output, args.n_best)
    korektor_evaluation.print_summary()
    print ""
    korektor_evaluation.evaluate()
    korektor_evaluation.print_results()



