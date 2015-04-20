#!/bin/bash

set -e

# Get the model
[ \! -d ../../models/korektor-czech-130202 ] && (cd ../../models && sh korektor-czech-130202.sh)

# Compile korektor.
make -C ../../src -j3 korektor

# Test various configurations.
#for test in {spellchecking_h2mor,spellchecking_h2mor_2edits}:{olga.dirty:olga.gold,capek.gold:capek.gold} diacritics_h2mor:capek.nodia:capek.gold; do
for test in spellchecking_h2mor:olga.dirty:olga.gold diacritics_h2mor:capek.nodia:capek.gold spellchecking_h2mor:capek.gold:capek.gold; do
  conf=${test%%:*}; test=${test#*:}
  data=${test%%:*}; test=${test#*:}
  gold=$test

  dataset=$conf:$data
  echo $dataset
  (time -p ../../src/korektor --input=vertical --output=vertical --corrections 3 "$@" ../../models/korektor-czech-130202/$conf.conf <eval/$data >eval/tmp) |& tr "\n" " "; echo
  perl eval.pl eval/tmp eval/$gold 3
  rm eval/tmp
  echo
done
