import re
import codecs
import sys

"""
====================
Korektor Evaluation
====================

Evaluation Measures
-------------------

The spellchecker accuracy is measured in terms of standard precision/recall are measured.
Precision/recall measures are calculated at two levels: (i) detection and (ii) correction.
For both levels, precision/recall and other related measures are calculated as follows:

|Precision      - TP / (TP + FP)
|Recall         - TP / (TP + FN)
|F-measure      - 2 * (precision * recall) / (precision + recall)
|Accuracy       - (TP + TN) / (TP + FP + TN + FN)

Error detection
---------------

|**TP** - Number of words with spelling errors that the spell checker detected correctly
|**FP** - Number of words identified as spelling errors that are not actually spelling errors
|**TN** - Number of correct words that the spell checker did not flag as having spelling errors
|**FN** - Number of words with spelling errors that the spell checker did not flag as having spelling errors

Error correction
----------------
|**TP** - Number of words with spelling errors for which the spell checker gave the correct suggestion
|**FP** - Number of words (with/without spelling errors) for which the spell checker made suggestions, and for those,
|         either the suggestion is not needed (in the case of non-existing errors) or the suggestion is incorrect if
|         indeed there was an error in the original word.
|**TN** - Number of correct words that the spell checker did not flag as having spelling errors and no
|         suggestions were made.
|**FN** - Number of words with spelling errors that the spell checker did not flag as having spelling errors or did not
|         provide any suggestions

"""

class SpellEval:
    def __init__(self, test_filename, gold_filename, output_filename, nbest):
        # for top-n accuracy (precision, recall etc)
        self.nbest = nbest

        # results : spelling error detection
        self.precision_d = 0.0
        self.recall_d = 0.0
        self.tp_d = 0
        self.fp_d = 0
        self.tn_d = 0
        self.fn_d = 0

        # results: spelling error correction
        self.precision_c = [0.0] * self.nbest
        self.recall_c = [0.0] * self.nbest
        self.tp_c = [0] * self.nbest
        self.fp_c = [0] * self.nbest
        self.tn_c = [0] * self.nbest
        self.fn_c = [0] * self.nbest

        self.corpus_size = 0
        self.gold_content = {}
        self.test_data_errors = {}
        self.system_suggestions = {}

        # open files for reading
        fH = self.open_files(test_filename, gold_filename, output_filename)

        test_file_lines = fH[0].readlines()
        gold_file_lines = fH[1].readlines()
        out_file_lines = fH[2].readlines()

        if len(test_file_lines) != len(gold_file_lines):
            print "error: file size differ between the test file and gold file"
            sys.exit(1)
        elif len(out_file_lines) != len(gold_file_lines):
            print "error: file size differ between the test file and gold file"
            sys.exit(1)

        self.close_files(fH)

        # some required overall and specific info
        (self.gold_content, self.corpus_size) = self.read_file_into_map(gold_file_lines)

        # each error entry is a tuple of sentence number and word number
        self.test_data_errors = self.get_error_locations(test_file_lines, gold_file_lines)

        # get suggestions from system output
        self.system_suggestions = self.get_suggestions_map(out_file_lines)


    def read_file_into_map(self, file_lines):
        i = 0
        total_words = 0
        content_map = {}
        while i < len(file_lines):
            line = file_lines[i]
            line = re.sub(r'\n$', '', line)
            words = re.split(r'\s+', line)
            total_words += len(words)
            j = 0
            while j < len(words):

                content_map[(i,j)] = words[j]
                j += 1
            i += 1
        return content_map, total_words

    def open_files(self, test_filename, gold_filename, output_filename):
        file_handles = []
        try:
            tfh = codecs.open(test_filename, "r", encoding="utf-8")
            gfh = codecs.open(gold_filename, "r", encoding="utf-8")
            ofh = codecs.open(output_filename, "r", encoding="utf-8")
        except IOError, v:
            print v
        else:
            file_handles = [tfh, gfh, ofh]

        return file_handles

    def get_error_locations(self, test_file_lines, gold_file_lines):
        # error_data[error_location] = (error, gold)
        error_data = {}
        i = 0
        while i < len(test_file_lines):
            test_line = test_file_lines[i]
            gold_line = gold_file_lines[i]
            test_line = re.sub(r'\n$', '', test_line)
            gold_line = re.sub(r'\n$', '', gold_line)
            test_words = re.split(r'\s+', test_line)
            gold_words = re.split(r'\s+', gold_line)

            if len(test_words) != len(gold_words):
                print 'error: the number of words in test sentence and gold sentence differ. Error line number - ', i
            j = 0
            while j < len(test_words):
                if test_words[j] != gold_words[j]:
                    error_data[(i,j)] = (test_words[j], gold_words[j])
                j += 1
            i += 1
        return error_data

    def get_error_detection_results(self):
        return self.precision_d, self.recall_d

    def get_error_correction_results(self):
        return self.precision_c, self.recall_c

    def evaluate(self):

        total_errors = len(self.test_data_errors)

        # fill/update true/false positives for correction/detection
        for err_loc in self.system_suggestions.keys():
            if err_loc in self.test_data_errors.keys():
                self.tp_d += 1
                word_suggestions = self.system_suggestions[err_loc]
                error_found = False
                i = 0;
                while i < len(self.system_suggestions[err_loc]):
                    if word_suggestions[i] == self.test_data_errors[err_loc][1]:
                        error_found = True
                        self.tp_c[i] += 1
                        j = i+1
                        while j < len(self.system_suggestions[err_loc]):
                            self.tp_c[j] += 1
                            j += 1
                        break
                    else:
                        self.fp_c[i] += 1
                    i += 1
                if len(self.system_suggestions[err_loc]) < self.nbest:
                        k = len(self.system_suggestions[err_loc])
                        while k < self.nbest:
                            if error_found:
                                self.tp_c[k] += 1
                            else:
                                self.fp_c[k] += 1
                            k += 1
            else:
                self.fp_d += 1
                m = 0
                while m < self.nbest:
                    self.fp_c[m] += 1
                    m += 1

        # fill/update true/false negatives for correction/detection
        self.tn_d = self.corpus_size - len(self.system_suggestions.keys())
        for test_err_loc in self.test_data_errors.keys():
            if not test_err_loc in self.system_suggestions.keys():
                self.tn_d -= 1
                self.fn_d += 1
        for i in range(self.nbest):
            self.fn_c[i] = self.fn_d

        for i in range(self.nbest):
            self.tn_c[i] = self.tn_d

        # precision/recall for error detection
        self.precision_d = (1.0 * self.tp_d) / (self.tp_d + self.fp_d)
        self.recall_d = (1.0 * self.tp_d) / (self.tp_d + self.fn_d)

        # calculate precision/recall for spelling correction
        for i in range(len(self.tp_c)):
            self.precision_c[i] = (1.0 * self.tp_c[i]) / (self.tp_c[i] + self.fp_c[i])
            self.recall_c[i] = (1.0 * self.tp_c[i]) / (self.tp_c[i] + self.fn_c[i])

    def get_suggestions_map(self, out_file_lines):
        i = 0
        suggestions_map = {}
        while i < len(out_file_lines):
            out_line = out_file_lines[i]
            out_line = re.sub(r'\n$', '', out_line)
            out_line = re.sub(r'\<spelling\s+original','<spellingoriginal', out_line)
            out_line = re.sub(r'\<grammar\s+original','<grammaroriginal', out_line)
            out_line = re.sub(r'\"\s+suggestions','"suggestions', out_line)

            out_words = re.split(r'\s+', out_line)
            j = 0
            while j < len(out_words):
                suggestion_match = re.search(r'\"suggestions=\"(.+)\"', out_words[j])
                if suggestion_match:
                    suggestion_line= suggestion_match.group(1)
                    suggestions = re.split(r'\|', suggestion_line)
                    suggestions_map[(i,j)] = suggestions
                j += 1
            i += 1
        return suggestions_map

    """
    Prints errors in the test data with gold information
    """
    def print_errors(self):
        for err_loc in self.test_data_errors:
            print err_loc, " => ", "original: ", self.test_data_errors[err_loc][0], ", gold: ", self.test_data_errors[err_loc][1]

    def print_results(self):
        print ''.rjust(20), "***************"
        print ''.rjust(20), "Error detection"
        print ''.rjust(20), "***************"
        print 'Precision'.ljust(10), ':', '{:5.2f}'.format(self.precision_d * 100.0)
        print 'Recall'.ljust(10), ':', '{:5.2f}'.format(self.recall_d * 100.0)
        print ""

        print ''.rjust(20), "***************"
        print ''.rjust(20), "Error correction"
        print ''.rjust(20), "***************"
        print ""

        print "top-n".ljust(6), 'Precision'.rjust(10), 'Recall'.rjust(10)
        print "-----".ljust(6), '---------'.rjust(10), '------'.rjust(10)
        i = 0
        while i < len(self.precision_c):
            top = 'top-' + str(i+1)
            print top.ljust(6), ''.rjust(4), '{:5.2f}'.format(self.precision_c[i]*100.0), ''.rjust(4), '{:5.2f}'.format(self.recall_c[i]*100.0)
            i += 1

    def close_files(self, file_handles):
        for fh in file_handles:
            fh.close()

