#!/usr/bin/env python

"""
Usage: ./create_data_from_cnc.py <input_file> <output_directory>
The program reads the input data in CNC format and outputs several files needed for
training models.

The following files are created in the <output_directory>

1. morphlex.txt - This file is used by korektor morphology tool for creating morphological lexicon.
                  Each line of the morphlex.txt has of the format - "form|lemma|POS"

2. freq.txt - This file contains frequency counts for the morphological lexicon. The format of the file is
                  "form|lemma|POS freq_count"

3. forms.txt - One sentence per line from the CNC corpus

4. lemmas.txt - One sentence per line from the CNC corpus, but lemmas instead of forms

5. tags.txt - Once sentence per line. The file contains tags for each form in the CNC corpus (forms.txt)

6. forms_tags.txt - Once sentence per line. This file contains both forms and tags and are delimited by '/'

"""

import codecs
import re
import sys

def write_data(filename, outputDir):

    # open files for writing
    fHandles = open_files_for_writing(filename, outputDir)

    # read CNC data
    try:
        fH = codecs.open(filename, 'rU', 'utf-8')
    except IOError as v:
        print v
    else:
        counts = {}
        sentence = []
        senOfLemmas = []
        senOfTags = []
        senOfFormTags = []
        senStarted = False

        # patterns in CNC file
        senPat = r'(.+)\s+(.+)\s+(.+)'
        senStartPat = r'<s\s+id=\"\d+\">'
        senEndPat = r'<\/s>'

        for line in fH:
            if re.search(senStartPat, line):
                senStarted = True
                continue
            if re.search(senEndPat, line) and senStarted:
                fHandles[2].write(' '.join(sentence) + '\n')
                fHandles[3].write(' '.join(senOfLemmas) + '\n')
                fHandles[4].write(' '.join(senOfTags) + '\n')
                fHandles[5].write(' '.join(senOfFormTags) + '\n')
                senStarted = False
                sentence = []
                senOfLemmas = []
                senOfTags = []
                senOfFormTags = []

            if senStarted:
                lineC = re.sub(r'\n$', r'', line)
                tokens = []
                senFound = re.search(senPat, lineC)
                if senFound:
                    # separate list for forms, lemmas, tags for each sentence
                    tokens.append(senFound.group(1))
                    tokens.append(senFound.group(2))
                    tokens.append(senFound.group(3))
                    sentence.append(tokens[0])
                    senOfLemmas.append(tokens[1])
                    senOfTags.append(tokens[2])
                    senOfFormTags.append(tokens[0] + '/' + tokens[2])

                    # frequency counts
                    tokTuple = (tokens[0], tokens[1], tokens[2])
                    tokTupleStr = '|'.join(tokTuple)
                    if  tokTupleStr in counts:
                        counts[tokTupleStr] += 1
                    else:
                        counts[tokTupleStr] = 1

    # write morphology lexicon and frequency counts
    fHandles[0].write('form|lemma|pos\n')
    fHandles[0].write('-----\n')
    for k in counts.keys():
        fHandles[0].write(k + '\n')
        fHandles[1].write(k + ' ' + str(counts[k]) + '\n')

    close_file_handles(fHandles)
    return

def open_files_for_writing(filename, outputDir):
    morphLexiconFile = outputDir + '/' + 'morphlex.txt'
    freqFile = outputDir + '/'  +  'freq.txt'
    sentencesFile = outputDir + '/'  + 'forms.txt'
    lemmasFile = outputDir + '/'  +  'lemmas.txt'
    tagsFile = outputDir + '/'  + 'tags.txt'
    formTagsFile = outputDir + '/'  + 'forms_tags.txt'
    fileHandles = []
    try:
        mLexH = codecs.open(morphLexiconFile, 'w', 'utf-8')
        freqH = codecs.open(freqFile, 'w', 'utf-8')
        senH = codecs.open(sentencesFile, 'w', 'utf-8')
        lemmasH = codecs.open(lemmasFile, 'w', 'utf-8')
        tagsH = codecs.open(tagsFile, 'w', 'utf-8')
        formTagsH = codecs.open(formTagsFile, 'w', 'utf-8')
        print 'Writing morphology lexicon to .... ' + morphLexiconFile
        print 'Writing frequency counts to .... ' + freqFile
        print 'Writing sentences to .... ' + sentencesFile
        print 'Writing lemmas to .... ' + lemmasFile
        print 'Writing tags to .... ' + tagsFile
    except IOError as v:
        print v
    else:
        fileHandles = [mLexH, freqH, senH, lemmasH, tagsH, formTagsH]

    return fileHandles

def close_file_handles(fHandles):
    for h in fHandles:
        h.close()
    return

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print 'Usage: ./create_data_from_cnc.py <input_file> <output_directory>'
        sys.exit(2)

    write_data(sys.argv[1], sys.argv[2])
