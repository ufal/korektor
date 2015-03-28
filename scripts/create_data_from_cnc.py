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

        blockSize = 10000 # write 10k sentences per write operation
        sentencesBlock = []
        senOfLemmasBlock =[]
        senOfTagsBlock= []
        senOfFormTagsBlock = []

        senStarted = False

        # patterns in CNC file
        senPat = r'(.+)\s+(.+)\s+(.+)'
        senStartPat = r'<s\s+id=\"\d+\">'
        senEndPat = r'<\/s>'

        linesRead = 0
        blockNumber = 1
        print '--> Reading the input file ... ' + filename
        for line in fH:
            if re.search(senStartPat, line):
                senStarted = True
                continue
            if re.search(senEndPat, line) and senStarted:
                linesRead += 1
                sentencesBlock.append(' '.join(sentence))
                senOfLemmasBlock.append(' '.join(senOfLemmas))
                senOfTagsBlock.append(' '.join(senOfTags))
                senOfFormTagsBlock.append(' '.join(senOfFormTags))
                if linesRead == blockSize:
                    fHandles[2].write('\n'.join(sentencesBlock) + '\n')
                    fHandles[3].write('\n'.join(senOfLemmasBlock) + '\n')
                    fHandles[4].write('\n'.join(senOfTagsBlock) + '\n')
                    fHandles[5].write('\n'.join(senOfFormTagsBlock) + '\n')
                    sentencesBlock = []
                    senOfLemmasBlock =[]
                    senOfTagsBlock= []
                    senOfFormTagsBlock = []
                    print '\t--> Read : ' + str(blockNumber * blockSize) + ' sentences'
                    blockNumber += 1
                    linesRead = 0
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

                    # replace '|' with '@@@' in forms and lemmas
                    tokens[0] = re.sub('\|', '@@@', tokens[0])
                    tokens[1] = re.sub('\|', '@@@', tokens[1])
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
        # unwritten stuff
        if sentencesBlock:
            fHandles[2].write('\n'.join(sentencesBlock) + '\n')
            fHandles[3].write('\n'.join(senOfLemmasBlock) + '\n')
            fHandles[4].write('\n'.join(senOfTagsBlock) + '\n')
            fHandles[5].write('\n'.join(senOfFormTagsBlock) + '\n')

    # write morphological lexicon and frequency counts
    blockSize = 100000 # 100000 words per write operation
    fHandles[0].write('form|lemma|pos\n')
    fHandles[1].write('form|lemma|pos\n')
    keyCountsList = []
    keysList = counts.keys()
    for k in counts.keys():
        keyCountsList.append(k + ' ' + str(counts[k]))

    print '\n\t--> Writing morphological lexicon and frequency counts'
    i = 0
    while i * blockSize < len(keysList):
        if (i+1) * blockSize > len(keysList):
            fHandles[0].write('\n'.join(keysList[(i * blockSize):]) + '\n')
            fHandles[1].write('\n'.join(keyCountsList[(i * blockSize):]) + '\n')
        else:
            fHandles[0].write('\n'.join(keysList[(i * blockSize): (i+1) * blockSize]) + '\n')
            fHandles[1].write('\n'.join(keyCountsList[(i * blockSize): (i+1) * blockSize]) + '\n')
        i += 1

    print 'Done'

    # close the input and output files
    fH.close()
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
        print '*'*60
        print 'Morphology lexicon will be written to .... ' + morphLexiconFile
        print 'Frequency counts will be written to .... ' + freqFile
        print 'Sentences  will be written to .... ' + sentencesFile
        print 'Lemmas will be written to .... ' + lemmasFile
        print 'Tags will be written to .... ' + tagsFile
        print 'Forms/tags  will be written to .... ' + formTagsFile
        print '*'*60
        print '\n'
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
