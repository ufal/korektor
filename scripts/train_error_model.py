# -*- coding: utf-8 -*-
"""Script for training the error model.

Reimplementation of the error model training as done in train_error_model Perl script by Michal Richter.

"""

import argparse
import codecs

class ErrorModel:
    """Basic error model.

    """
    all_letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZĚŠČŘŽÝÁÍÉÓĎŤŇÚÚabcdefghijklmnopqrstuvwxyzěščřžýáíéóďťňúů"
    letters_pos_diac = "ESCRZYAIODTNUescrzyaiodtnu"
    letters_diac = "ĚŠČŘŽÝÁÍÉÓĎŤŇÚÚěščřžýáíéóďťňúů"
    letters_vocal = "AEIOUYÁÉÍÓÚÝaeiouyáéíóúý"

    keyboard_rows = ["ěščřžýáíé", "qwertyuiopú", "asdfghjklů", "zxcvbnm"]
    keyboard_cols = ["qaz", "ěwsx", "šedc", "črfv", "řtgb", "žyhn", "ýujm", "áik", "íol", "épů"]
    keyboard_extra_vadj = {"sz" : 1, "dx" : 1, "fc" : 1, "gv" : 1, "hb" : 1, "jn" : 1, "km" : 1}

    def __init__(self, error_file):
        """Constructor.

        """
        self.error_file = error_file

        # initialize variables
        self.num_letters = 0
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

        self.read_error_data()

    def read_error_data(self):
        with codecs.open(self.error_file, mode='r', encoding='utf-8') as f:
            self.raw_lines = f.readlines()

    def create_model(self):
        for line in self.raw_lines:
            self.count_letters(line)
        return

    def count_letters(self, input_str):
        """Updates letter counts for different letter groups.
        """
        for i in range(len(input_str)):
            letter = input_str[i]
            if i < len(input_str)-1 and input_str[i] == input_str[i+1]:
                self.num_same_letter_twice += 1

            if letter in ErrorModel.all_letters:
                self.num_letters += 1
            if letter in ErrorModel.letters_pos_diac:
                self.num_pos_diac += 1
            if letter in ErrorModel.letters_diac:
                self.num_diac += 1
            if letter in ErrorModel.letters_vocal:
                self.num_vocal += 1

            if letter == 'i' or letter == 'í' or letter == 'I' or letter == 'Í':
                self.num_i += 1
            if letter == 'y' or letter == 'ý' or letter == 'Y' or letter == 'Ý':
                self.num_y += 1
            if letter == 's' or letter == 'S':
                self.num_s += 1
            if letter == 'z' or letter == 'Z':
                self.num_z += 1
            if letter == 'n' or letter == 'N':
                self.num_n += 1

            has_hadj_neighbor = False
            has_vadj_neighbor = False

            if i > 0:
                if self.letters_hadj(letter, input_str[i-1]):
                    has_hadj_neighbor = True
                if self.letters_vadj(letter, input_str[i-1]):
                    has_vadj_neighbor = True
            if i < len(input_str)-1:
                if self.letters_hadj(letter, input_str[i+1]):
                    has_hadj_neighbor = True
                if self.letters_vadj(letter, input_str[i+1]):
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


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Script for training the error model for Korektor')
    parser.add_argument('error_file', help='Text file containing errors')
    args = parser.parse_args()
    error_model1 = ErrorModel(args.error_file)