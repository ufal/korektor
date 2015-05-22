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
import unicodedata

class ErrorModel:
    """Basic error model.

    """
    all_letters = u'ABCDEFGHIJKLMNOPQRSTUVWXYZĚŠČŘŽÝÁÍÉÓĎŤŇÚŮabcdefghijklmnopqrstuvwxyzěščřžýáíéóďťňúů'
    letters_pos_diac = u'ESCRZYAIODTNUescrzyaiodtnu'
    letters_diac = u'ĚŠČŘŽÝÁÍÉÓĎŤŇÚÚěščřžýáíéóďťňúů'
    letters_vocal = u'AEIOUYÁÉÍÓÚÝaeiouyáéíóúý'

    code_points_diacritics = {
        u'\u0301' : 1, # as in 'ý'
        u'\u030c' : 1, # as in 'š'
    }

    keyboard_rows = [u'ěščřžýáíé', u'qwertyuiopú', u'asdfghjklů', u'zxcvbnm']
    keyboard_cols = [u'qaz', u'ěwsx', u'šedc', u'črfv', u'řtgb', u'žyhn', u'ýujm', u'áik', u'íol', u'épů']
    keyboard_extra_vadj = {'sz' : 1, 'dx' : 1, 'fc' : 1, 'gv' : 1, 'hb' : 1, 'jn' : 1, 'km' : 1}

    def __init__(self, error_file, model_file):
        """Constructor.

        """
        self.error_file = error_file
        self.model_file = model_file

        # letter/biletter counts
        self.num_letters = 0
        self.letter_count = collections.defaultdict(int)
        self.num_biletters = 0
        self.biletter_count = collections.defaultdict(int)

        # other counts
        self.num_pos_diac = 0
        self.num_diac = 0
        self.num_vocal = 0
        self.num_has_hadj_neighbour = 0
        self.num_has_vadj_neighbour = 0
        self.num_has_random_neighbours = 0
        self.num_same_letter_twice = 0

        self.case_errors = 0
        self.diac_errors = 0

        # Confusion set for different edit operations
        self.cm_del = collections.defaultdict(int)
        self.cm_add = collections.defaultdict(int)
        self.cm_sub = collections.defaultdict(int)
        self.cm_rev = collections.defaultdict(int)

        # Error model probabilities for different edit operations
        self.prob_del = collections.defaultdict(int)
        self.prob_add = collections.defaultdict(int)
        self.prob_sub = collections.defaultdict(int)
        self.prob_rev = collections.defaultdict(int)

        # general cost
        self.case_cost = 0.0
        self.sub_cost = 0.0
        self.add_cost = 0.0
        self.del_cost = 0.0
        self.rev_cost = 0.0

        self.read_error_data()
        self.create_model()
        self.write_model()

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
                        if typo.lower() == correct.lower():
                            self.case_errors += 1
                        if is_chars_diff_by_diacritic(typo, correct):
                            self.diac_errors += 1
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
                    self.count_letters(spell_err[3])
                line_wo_err = re.sub(r'<type=\"((swap|s|i|d)_.+?)\"\sorig=\"(.+?)\"\sgold=\"(.+?)\">', '', line)
                self.count_letters(line_wo_err)
            else:
                self.count_letters(line)

        # calculate log probabilities
        for edit_key in self.cm_del:
            if float(self.biletter_count[edit_key]) != 0.0:
                err_prob = self.cm_del[edit_key] / float(self.biletter_count[edit_key])
                #print 'Error prob: \t' + str(err_prob)
                err_logprob = -math.log10(err_prob)
                self.prob_del[edit_key] = err_logprob
            else:
                print 'Warning: division by zero (del[' \
                      + edit_key.encode('utf-8') +']/bigram['+ edit_key.encode('utf-8') + '],\t' \
                      + repr(self.cm_del[edit_key]) + '/' + repr(self.biletter_count[edit_key]) + ')'
        for edit_key in self.cm_rev:
            if float(self.biletter_count[edit_key]) != 0.0:
                err_prob = self.cm_rev[edit_key] / float(self.biletter_count[edit_key])
                err_logprob = -math.log10(err_prob)
                self.prob_rev[edit_key] = err_logprob
            else:
                print 'Warning: division by zero (rev[' \
                      + edit_key.encode('utf-8') +']/bigram['+ edit_key.encode('utf-8') + '],\t' \
                      + repr(self.cm_rev[edit_key]) + '/' + repr(self.biletter_count[edit_key]) + ')'
        for edit_key in self.cm_sub:
            denom_key = edit_key[1]
            if float(self.letter_count[denom_key]) != 0.0:
                err_prob = self.cm_sub[edit_key] / float(self.letter_count[denom_key])
                err_logprob = -math.log10(err_prob)
                self.prob_sub[edit_key] = err_logprob
            else:
                print 'Warning: division by zero (sub[' \
                      + edit_key.encode('utf-8') +']/bigram['+ denom_key.encode('utf-8') + '],\t' \
                      + repr(self.cm_sub[edit_key]) + '/' + repr(self.letter_count[denom_key]) + ')'
        for edit_key in self.cm_add:
            denom_key = edit_key[1:3]
            if float(self.biletter_count[denom_key]) != 0.0:
                err_prob = self.cm_add[edit_key] / float(self.biletter_count[denom_key])
                err_logprob = -math.log10(err_prob)
                self.prob_add[edit_key] = err_logprob
            else:
                print 'Warning: division by zero (add[' \
                      + edit_key.encode('utf-8') +']/bigram['+ denom_key.encode('utf-8') + '],\t' \
                      + repr(self.cm_add[edit_key]) + '/' + repr(self.biletter_count[denom_key]) + ')'

        # calculate general costs
        self.case_cost = -math.log10((self.case_errors + 1.0) / self.num_letters)
        self.sub_cost =  -math.log10((len(self.cm_sub) + 1.0) / self.num_letters)
        self.add_cost = -math.log10((len(self.cm_add) + 1.0) / self.num_biletters)
        self.del_cost = -math.log10((len(self.cm_del) + 1.0) / self.num_biletters)
        self.rev_cost = -math.log10((len(self.cm_rev) + 1.0) / self.num_biletters)

        return

    def write_model(self):
        """Write model output to file.

        """
        with codecs.open(self.model_file, mode='w', encoding='utf-8') as f:
            f.write('case\t0\t' + repr(self.case_cost)+'\n')
            f.write('substitutions\t1\t' + repr(self.sub_cost)+'\n')
            f.write('insertions\t1\t' + repr(self.add_cost)+'\n')
            f.write('deletions\t1\t' + repr(self.del_cost)+'\n')
            f.write('swaps\t1\t' + repr(self.rev_cost)+'\n')

            for edit_key in self.prob_del:
                edit_key_c = edit_key[1] + edit_key[0]
                edit_key_c = re.sub('[\^|\$]', ' ', edit_key)
                f.write('d_' + edit_key_c + '\t1\t'+ repr(self.prob_del[edit_key])+'\n')
            for edit_key in self.prob_sub:
                edit_key_c = re.sub('[\^|\$]', ' ', edit_key)
                dist = 1
                if edit_key[0].lower() == edit_key[1].lower():
                    dist = 0
                if is_chars_diff_by_diacritic(edit_key[0], edit_key[1]):
                    dist = 0
                f.write('s_' + edit_key_c + '\t' + repr(dist) + '\t'+ repr(self.prob_sub[edit_key])+'\n')
            for edit_key in self.prob_rev:
                edit_key_c = re.sub('[\^|\$]', ' ', edit_key)
                f.write('swap_' + edit_key_c + '\t1\t'+ repr(self.prob_rev[edit_key])+'\n')
            for edit_key in self.prob_add:
                edit_key_c = re.sub('[\^|\$]', ' ', edit_key)
                f.write('i_' + edit_key_c + '\t1\t'+ repr(self.prob_add[edit_key])+'\n')

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
                    self.num_biletters += 1
                if i == len(w)-1:
                    self.biletter_count[w[i]+'$'] += 1
                    self.num_biletters += 1
                if i+1 < len(w):
                    self.biletter_count[w[i:i+2]] += 1
                    self.num_biletters += 1

                if i < len(w)-1 and w[i] == w[i+1]:
                    self.num_same_letter_twice += 1
                if letter in ErrorModel.letters_pos_diac:
                    self.num_pos_diac += 1
                if letter in ErrorModel.letters_diac:
                    self.num_diac += 1
                if letter in ErrorModel.letters_vocal:
                    self.num_vocal += 1

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
        print '\n'
        print '*************************'
        print 'Error model probabilities'
        print '*************************'
        print 'case\t0\t' + repr(self.case_cost)
        print 'substitutions\t1\t' + repr(self.sub_cost)
        print 'insertions\t1\t' + repr(self.add_cost)
        print 'deletions\t1\t' + repr(self.del_cost)
        print 'swaps\t1\t' + repr(self.rev_cost)

        for edit_key in self.prob_del:
            edit_key_c = edit_key[1] + edit_key[0]
            print 'd_' + edit_key_c.encode('utf-8') + '\t1\t'+ repr(self.prob_del[edit_key])
        for edit_key in self.prob_sub:
            dist = 1
            if edit_key[0].lower() == edit_key[1].lower():
                dist = 0
            if is_chars_diff_by_diacritic(edit_key[0], edit_key[1]):
                dist = 0
            print 's_' + edit_key.encode('utf-8') + '\t' + repr(dist) + '\t'+ repr(self.prob_sub[edit_key])
        for edit_key in self.prob_rev:
            print 'swap_' + edit_key.encode('utf-8') + '\t1\t'+ repr(self.prob_rev[edit_key])
        for edit_key in self.prob_add:
            print 'i_' + edit_key.encode('utf-8') + '\t1\t'+ repr(self.prob_add[edit_key])


def is_chars_diff_by_diacritic(char1, char2):
    """Check whether two characters differ only by diacritic.

    """
    if isinstance(char1, str):
        char1 = unicode(char1, encoding='utf-8')
    if isinstance(char2, str):
        char2 = unicode(char2, encoding='utf-8')
    char1_d = unicodedata.normalize('NFD', char1)
    char2_d = unicodedata.normalize('NFD', char2)
    if len(char1_d) == 1 and len(char2_d) == 2:
        if char1_d[0] == char2_d[0] and char2_d[1] in ErrorModel.code_points_diacritics:
            return True
    elif len(char1_d) == 2 and len(char2_d) == 1:
        if char1_d[0] == char2_d[0] and char1_d[1] in ErrorModel.code_points_diacritics:
            return True
    else:
        return False

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Script for training the error model for Korektor')
    parser.add_argument('error_file', help='Text file containing errors')
    parser.add_argument('model_file', help='Filename where the model output should be written')
    args = parser.parse_args()
    error_model1 = ErrorModel(args.error_file, args.model_file)
    error_model1.print_error_model()