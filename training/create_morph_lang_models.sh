#!/bin/bash
#
# This script creates binary morphology and language models for 
# Korektor.
# 
# Language models are created using KenLM. KenLM binaries should be
# on the search path.
#
# Usage:
#
# ./create_morph_lang_models.sh <forms file> <lemmas file> <tags file> <freq file> <LM order> <output director> <Korektor home>
#
#
#


## The following files should have the format of one sentence per line.
## The number of tokens in each file should be same.

FORMS_FILE=$1
LEMMAS_FILE=$2
TAGS_FILE=$3
FREQ_FILE=$4
NGRAM_ORDER=$5

## Where the output files should go
OUTPUT_DIR=$6

## !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
## Set Korektor home directory
## !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
KOREKTOR_HOME=$7
SCRIPTS_DIR=${KOREKTOR_HOME}/scripts

## Temp dir
TMP_DIR=`pwd`/tmp_lm

# binarize morphological lexixon for Korektor
${KOREKTOR_HOME}/src/create_morphology ${FREQ_FILE} ${OUTPUT_DIR}/morphlex.bin ${OUTPUT_DIR}/vocab.bin ${OUTPUT_DIR}/test.bin

# language models using KenLM
lmplz -o ${NGRAM_ORDER} -S 20% -T ${TMP_DIR} < ${FORMS_FILE} > ${OUTPUT_DIR}/forms.arpa
lmplz -o ${NGRAM_ORDER} -S 20% -T ${TMP_DIR} < ${LEMMAS_FILE} > ${OUTPUT_DIR}/lemmas.arpa
lmplz -o ${NGRAM_ORDER} -S 20% -T ${TMP_DIR} < ${TAGS_FILE} > ${OUTPUT_DIR}/tags.arpa

## binarize LMs for Korektor
${KOREKTOR_HOME}/src/create_lm_binary ${OUTPUT_DIR}/forms.arpa ${OUTPUT_DIR}/morphlex.bin ${OUTPUT_DIR}/vocab.bin "form" ${NGRAM_ORDER} ${OUTPUT_DIR}/forms.arpa.kor.bin
${KOREKTOR_HOME}/src/create_lm_binary ${OUTPUT_DIR}/lemmas.arpa ${OUTPUT_DIR}/morphlex.bin ${OUTPUT_DIR}/vocab.bin "lemma" ${NGRAM_ORDER} ${OUTPUT_DIR}/lemmas.arpa.kor.bin
${KOREKTOR_HOME}/src/create_lm_binary ${OUTPUT_DIR}/tags.arpa ${OUTPUT_DIR}//morphlex.bin ${OUTPUT_DIR}/vocab.bin "pos" ${NGRAM_ORDER} ${OUTPUT_DIR}/tags.arpa.kor.bin


