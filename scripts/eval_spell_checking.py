#!/usr/bin/env
import argparse
import codecs
import re
import sys


def eval_spell_checking(testFilename, goldFilename, outputFilename):
    accuracy = []

    # open files for reading and writing
    fH = open_files(testFilename, goldFilename, outputFilename)

    testFileLines = fH[0].readlines()
    goldFileLines = fH[1].readlines()
    outFileLines = fH[2].readlines()
    if len(testFileLines) != len(goldFileLines):
        print "error: file size differ between the test file and gold file"
        sys.exit(1)
    elif len(outFileLines) != len(goldFileLines):
        print "error: file size differ between the test file and gold file"
        sys.exit(1)

    # some required overall and specific info
    (goldFileContentMap, totalNumWords) = read_gold_data_into_map(goldFileLines)

    # each entry is a tuple of sentence number and word number
    errorIndexBetGoldAndTest = []
    errorIndexBetGoldAndTest = get_error_locations(testFileLines, goldFileLines)

    return accuracy

def read_gold_data_into_map(goldFileLines):
    i = 0
    totalWords = 0
    goldFileContentMap = {}
    while i < len(goldFileLines):
        goldLine = goldFileLines[i]
        goldLine = re.sub(r'\n$', '', goldLine)
        goldWords = re.split(r'\s+', goldLine)
        j = 0
        while j < len(goldWords):
            totalWords += len(goldWords)
            goldFileContentMap[(i,j)] = goldWords[j]
            j += 1
        i += 1
    return (goldFileContentMap, totalWords)

def open_files(testFilename, goldFilename, outputFilename):
    fHandles = []
    try:
        tfH = codecs.open(testFilename, "r", encoding="utf-8")
        gfH = codecs.open(goldFilename, "r", encoding="utf-8")
        ofH = codecs.open(outputFilename, "w", encoding="utf-8")
    except IOError, v:
        print v
    else:
        fHandles = [tfH, gfH, ofH]

    return fHandles

def get_error_locations(testFileLines, goldFileLines):
    errorLocations = []
    errorLocGoldWordMap = {}
    totalWords = 0
    i = 0
    while i < len(testFileLines):
        testLine = testFileLines[i]
        goldLine = goldFileLines[i]
        testLine = re.sub(r'\n$', '', testLine)
        goldLine = re.sub(r'\n$', '', goldLine)
        testWords = re.split(r'\s+', testLine)
        goldWords = re.split(r'\s+', goldLine)

        if len(testWords) != len(goldWords):
            print 'error: the number of words in test sentence and gold sentence differ. Error line number - ', i
        j = 0
        while j < len(testWords):
            if testWords[j] != goldWords[j]:
                errorLocations.append((i,j))
            j += 1
        totalWords += len(testWords)
        i += 1
    return errorLocations


def close_files(fHandles):
    for fh in fHandles:
        fh.cose()

if __name__ == '__main__':

    # parser the command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("test_file", help="Test file in UTF-8")
    parser.add_argument("gold_file", help="Gold file in UTF-8")
    parser.add_argument("system_output", help="System output (Korektor)")
    args = parser.parse_args()

    eval_spell_checking(args.test_file, args.gold_file, args.system_output)


