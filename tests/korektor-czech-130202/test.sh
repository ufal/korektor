#!/bin/sh

set -e

# Get the model and update the config files if needed.
if [ \! -d korektor-czech-130202 ]; then
  curl -O https://lindat.mff.cuni.cz/repository/xmlui/bitstream/handle/11234/1-1460/korektor-czech-130202.zip
  unzip korektor-czech-130202.zip
  rm korektor-czech-130202.zip

  sed 's/mode=tag_errors/mode=autocorrect/' -i korektor-czech-130202/*.conf
fi

# Compile korektor.
make -C ../../src korektor -j4 korektor

# Test various configurations.
for conf in diacritics_h2mor spellchecking_h2mor spellchecking_h2mor_2edits; do
  echo Testing $conf model... >&2
  ../../src/korektor korektor-czech-130202/$conf.conf <povidky_z_jedne_kapsy.nodia | diff -u povidky_z_jedne_kapsy.$conf -
done
