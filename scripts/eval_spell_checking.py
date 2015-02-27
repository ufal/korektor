#!/usr/bin/env
import argparse
import codecs
import re
import sys



def eval_spell_checking(test_filename, gold_filename, output_filename):
    # open files for reading
    fH = open_files(test_filename, gold_filename, output_filename)

    test_file_lines = fH[0].readlines()
    gold_file_lines = fH[1].readlines()
    out_file_lines = fH[2].readlines()
    if len(test_file_lines) != len(gold_file_lines):
        print "error: file size differ between the test file and gold file"
        sys.exit(1)
    elif len(out_file_lines) != len(gold_file_lines):
        print "error: file size differ between the test file and gold file"
        sys.exit(1)

    # some required overall and specific info
    (gold_file_content_map, total_num_words) = read_gold_data_into_map(gold_file_lines)

    # each error entry is a tuple of sentence number and word number
    error_index_bet_gold_and_test = get_error_locations(test_file_lines, gold_file_lines)

    # get suggestions from system output
    suggestions_map = get_suggestions_map(out_file_lines)

    # calculate the error
    calculate_error_statistics(gold_file_content_map, error_index_bet_gold_and_test, suggestions_map)


def read_gold_data_into_map(gold_file_lines):
    i = 0
    total_words = 0
    gold_file_content_map = {}
    while i < len(gold_file_lines):
        gold_line = gold_file_lines[i]
        gold_line = re.sub(r'\n$', '', gold_line)
        gold_words = re.split(r'\s+', gold_line)
        j = 0
        while j < len(gold_words):
            total_words += len(gold_words)
            gold_file_content_map[(i,j)] = gold_words[j]
            j += 1
        i += 1
    return (gold_file_content_map, total_words)

def open_files(test_filename, gold_filename, output_filename):
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

def get_error_locations(test_file_lines, gold_file_lines):
    error_locations = []
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
                error_locations.append((i,j))
            j += 1
        i += 1
    return error_locations

def calculate_error_statistics(gold_file_content_map, true_error_locations, system_suggestions):
    total_errors = len(true_error_locations)
    num_relevant_sugg = 0
    true_tokens_in_N_sugg = [0, 0, 0, 0, 0]
    for err_loc in system_suggestions.keys():
        if err_loc in true_error_locations:
            num_relevant_sugg += 1
            word_suggestions = system_suggestions[err_loc]
            i = 0;
            while (i < 5) and (i < len(system_suggestions[err_loc])):
                if word_suggestions[i] == gold_file_content_map[err_loc]:
                    j = i
                    while j < 5:
                        true_tokens_in_N_sugg[j] += 1
                        j += 1
                i += 1

    correct_percentage = [ (nums_corr / (total_errors * 1.0)) * 100.0 for nums_corr in true_tokens_in_N_sugg]
    print "Total errors found in the test corpus: ", total_errors
    print "Number of relevant suggestions from the system: ", num_relevant_sugg
    print "True tokens found in N suggestions (1-5)  : " , true_tokens_in_N_sugg
    print "Percentage of correct suggestions in N system suggestions (1-5): ", correct_percentage


def get_suggestions_map(out_file_lines):
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

def close_files(file_handles):
    for fh in file_handles:
        fh.cose()

if __name__ == '__main__':

    # parser the command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("test_file", help="Test file in UTF-8")
    parser.add_argument("gold_file", help="Gold file in UTF-8")
    parser.add_argument("system_output", help="System output (Korektor)")
    args = parser.parse_args()

    eval_spell_checking(args.test_file, args.gold_file, args.system_output)