#!/bin/sh

set -e

# Get the model
[ \! -d ../../models/korektor-czech-130202 ] && (cd ../../models && sh korektor-czech-130202.sh)

# Compile korektor.
make -C ../../src -j3 korektor

# Test various configurations.
for conf in diacritics_h2mor spellchecking_h2mor spellchecking_h2mor_2edits; do
  echo Testing $conf model... >&2
  ../../src/korektor --input=untokenized_lines ../../models/korektor-czech-130202/$conf.conf <povidky_z_jedne_kapsy.nodia | diff -u povidky_z_jedne_kapsy.$conf -
done
