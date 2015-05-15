# -*- coding: utf-8 -*-
"""Script for training the error model.

Reimplementation of the error model training as done in train_error_model Perl script by Michal Richter.

"""

import argparse
import codecs
import re
import collections
import math
import sys

class ErrorModel:
    """Basic error model.

    """
    all_letters = u'ABCDEFGHIJKLMNOPQRSTUVWXYZĚŠČŘŽÝÁÍÉÓĎŤŇÚÚabcdefghijklmnopqrstuvwxyzěščřžýáíéóďťňúů'
    letters_pos_diac = u'ESCRZYAIODTNUescrzyaiodtnu'
    letters_diac = u'ĚŠČŘŽÝÁÍÉÓĎŤŇÚÚěščřžýáíéóďťňúů'
    letters_vocal = u'AEIOUYÁÉÍÓÚÝaeiouyáéíóúý'

    keyboard_rows = [u'ěščřžýáíé', u'qwertyuiopú', u'asdfghjklů', u'zxcvbnm']
    keyboard_cols = [u'qaz', u'ěwsx', u'šedc', u'črfv', u'řtgb', u'žyhn', u'ýujm', u'áik', u'íol', u'épů']
    keyboard_extra_vadj = {'sz' : 1, 'dx' : 1, 'fc' : 1, 'gv' : 1, 'hb' : 1, 'jn' : 1, 'km' : 1}

    def __init__(self, error_file):
        """Constructor.

        """
        self.error_file = error_file

        # letter counts
        self.num_letters = 0
        self.letter_count = collections.defaultdict(int)
        self.biletter_count = collections.defaultdict(int)
        self.triletter_count = collections.defaultdict(int)

        # other counts
        self.num_pos_diac = 0
        self.num_diac = 0
        self.num_i = 0
        self.num_y = 0
        self.num_s = 0
        self.num_z = 0
        self.num_vocal = 0
        self.num_n = 0
        self.num_has_hadj_neighbour = 0
        self.num_has_vadj_neighbour = 0
        self.num_has_random_neighbours = 0
        self.num_same_letter_twice = 0

        # Confusion set for different edit operations
        self.cm_del = collections.defaultdict(int)
        self.cm_add = collections.defaultdict(int)
        self.cm_sub = collections.defaultdict(int)
        self.cm_rev = collections.defaultdict(int)

        self.prob_del = collections.defaultdict(int)
        self.prob_add = collections.defaultdict(int)
        self.prob_sub = collections.defaultdict(int)
        self.prob_rev = collections.defaultdict(int)

        self.read_error_data()
        self.create_model()

    def read_error_data(self):
        """Read the text file containing containing spelling errors.

        """
        with codecs.open(self.error_file, mode='r', encoding='utf-8') as f:
            self.raw_lines = f.readlines()

    def create_model(self):
        """Creates error model from a set of sentences marked with spelling errors.

        """
        for line in self.raw_lines:
            error_matches = re.findall(r'<type=\"((swap|s|i|d)_.+?)\"\sorig=\"(.+?)\"\sgold=\"(.+?)\">', line)
            if error_matches:
                for spell_err in error_matches:
                    if re.search(r'swap\_', spell_err[0]):
                        correct_i =  spell_err[0][6]
                        correct_i1 = spell_err[0][5]
                        self.cm_rev[correct_i + correct_i1] += 1
                    elif re.search(r's\_', spell_err[0]):
                        # typo
                        typo =  spell_err[0][2]
                        # the correct character at the place of typo
                        correct = spell_err[0][3]
                        self.cm_sub[typo+correct] += 1
                    elif re.search(r'i\_', spell_err[0]):
                        # inserted typo
                        typo = spell_err[0][2]
                        # character to the left of the typo
                        correct_prev = spell_err[0][3]
                        # character to the right of the typo
                        correct_next = spell_err[0][4]
                        self.cm_add[typo+correct_prev+correct_next] += 1
                    elif re.search(r'd\_', spell_err[0]):
                        # deleted character from the correct word
                        deleted_char = spell_err[0][2]
                        # the character to the left of the deleted character
                        correct_prev = spell_err[0][3]
                        self.cm_del[correct_prev+deleted_char] += 1

                    self.count_letters(spell_err[2])
                line_wo_err = re.sub(r'<type=\"((swap|s|i|d)_.+?)\"\sorig=\"(.+?)\"\sgold=\"(.+?)\">', '', line)
                self.count_letters(line_wo_err)
            else:
                self.count_letters(line)

        self.print_confusion_sets()
        sys.exit()

        # calculate log probabilities
        for edit_key in self.cm_del:
            print 'Edit key: ' + edit_key
            err_prob = self.cm_del[edit_key] / self.biletter_count[edit_key]
            err_logprob = math.log10(err_prob)
            self.prob_del[edit_key] = err_logprob

        for edit_key in self.cm_rev:
            print 'Edit key: ' + edit_key
            err_prob = self.cm_rev[edit_key] / self.biletter_count[edit_key]
            err_logprob = math.log10(err_prob)
            self.prob_rev[edit_key] = err_logprob

        for edit_key in self.cm_sub:
            denom_key = edit_key[1]
            print 'Edit key: ' + edit_key + ', \tDenom Key: ' + denom_key
            err_prob = self.cm_rev[edit_key] / self.letter_count[denom_key]
            err_logprob = math.log10(err_prob)
            self.prob_sub[edit_key] = err_logprob

        for edit_key in self.cm_add:
            denom_key = edit_key[1:2]
            print 'Edit key: ' + edit_key + ', \tDenom Key: ' + denom_key
            err_prob = self.cm_rev[edit_key] / self.biletter_count[denom_key]
            err_logprob = math.log10(err_prob)
            self.prob_add[edit_key] = err_logprob
        return

    def count_letters(self, input_str):
        """Updates letter counts for different letter groups.

        """
        words = input_str.split()
        for w in words:
            for i in range(len(w)):
                letter = w[i]

                if letter in ErrorModel.all_letters:
                    self.num_letters += 1
                    self.letter_count[letter] += 1

                if i == 0:
                    self.biletter_count['^'+w[i]] += 1
                if i == len(w)-1:
                    self.biletter_count[w[i]+'$'] += 1
                if i+1 < len(w):
                    self.biletter_count[w[i:i+1]] += 1
                if i+2 < len(w):
                    self.triletter_count[w[i:i+2]] += 1


                if i < len(w)-1 and w[i] == w[i+1]:
                    self.num_same_letter_twice += 1
                if letter in ErrorModel.letters_pos_diac:
                    self.num_pos_diac += 1
                if letter in ErrorModel.letters_diac:
                    self.num_diac += 1
                if letter in ErrorModel.letters_vocal:
                    self.num_vocal += 1

                # if letter == 'i' or letter == 'í' or letter == 'I' or letter == 'Í':
                #     self.num_i += 1
                # if letter == 'y' or letter == 'ý' or letter == 'Y' or letter == 'Ý':
                #     self.num_y += 1
                # if letter == 's' or letter == 'S':
                #     self.num_s += 1
                # if letter == 'z' or letter == 'Z':
                #     self.num_z += 1
                # if letter == 'n' or letter == 'N':
                #     self.num_n += 1

                has_hadj_neighbor = False
                has_vadj_neighbor = False

                if i > 0:
                    if self.letters_hadj(letter, w[i-1]):
                        has_hadj_neighbor = True
                    if self.letters_vadj(letter, w[i-1]):
                        has_vadj_neighbor = True
                if i < len(w)-1:
                    if self.letters_hadj(letter, w[i+1]):
                        has_hadj_neighbor = True
                    if self.letters_vadj(letter, w[i+1]):
                        has_vadj_neighbor = True
                if has_hadj_neighbor:
                    self.num_has_hadj_neighbour += 1
                elif has_vadj_neighbor:
                    self.num_has_vadj_neighbour += 1
                else:
                    self.num_has_random_neighbours += 1
        return

    def letters_hadj(self, l1, l2):
        for row in self.keyboard_rows:
            l1_ind = row.find(l1)
            l2_ind = row.find(l2)
            if l1_ind >= 0 and l2_ind >= 0:
                if abs(l1_ind - l2_ind) == 1:
                    return True
        return False

    def letters_vadj(self, l1, l2):
        if l1+l2 in ErrorModel.keyboard_extra_vadj or l2+l1 in ErrorModel.keyboard_extra_vadj:
            return True
        for col in self.keyboard_cols:
            l1_ind = col.find(l1)
            l2_ind = col.find(l2)
            if l1_ind >= 0 and l2_ind >=0:
                if abs(l1_ind - l2_ind) == 1:
                    return True
        return False

    def print_confusion_sets(self):
        """Print the confusion set for misspellings that occurred by a single edit operation.

        """
        print "Substitution"
        print "============"
        for edit_key in self.cm_sub:
            print 'count("'+ edit_key.encode('utf-8') + '")\t' + repr(self.cm_sub[edit_key])

        print "\n\nDeletion"
        print "============"
        for edit_key in self.cm_del:
            print 'count("'+ edit_key.encode('utf-8') + '")\t' + repr(self.cm_del[edit_key])

        print "\n\nInsertion"
        print "============"
        for edit_key in self.cm_add:
            print 'count("'+ edit_key.encode('utf-8') + '")\t' + repr(self.cm_add[edit_key])

        print "\n\nSwap"
        print "============"
        for edit_key in self.cm_rev:
            print 'count("'+ edit_key.encode('utf-8') + '")\t' + repr(self.cm_rev[edit_key])
        return

    def print_error_model(self):
        """Print the error model.

        """
        for edit_key in self.prob_del:
            print 'd_' + edit_key + '\t1\t'+ self.prob_del[edit_key]
        for edit_key in self.prob_sub:
            print 's_' + edit_key + '\t1\t'+ self.prob_sub[edit_key]
        for edit_key in self.prob_rev:
            print 'swap_' + edit_key + '\t1\t'+ self.prob_rev[edit_key]
        for edit_key in self.prob_add:
            print 'i_' + edit_key + '\t1\t'+ self.prob_add[edit_key]


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Script for training the error model for Korektor')
    parser.add_argument('error_file', help='Text file containing errors')
    args = parser.parse_args()
    error_model1 = ErrorModel(args.error_file)
    error_model1.print_error_model()