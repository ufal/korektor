#!/bin/bash

set -e

# Get the model
[ \! -d ../../models/korektor-czech-130202 ] && (cd ../../models && sh korektor-czech-130202.sh)

# Compile korektor using release build
make -C ../../src korektor MODE=release -j4 korektor

# Test various configurations.
echo Date: `date` >&2
echo Hostname: `hostname` >&2
echo Current revision: `git rev-parse HEAD` >&2
for conf_num in diacritics_h2mor:30 spellchecking_h2mor:4 spellchecking_h2mor_2edits:1; do
  conf=${conf_num%:*}
  num=${conf_num##*:}
  echo Benchmarking $num-times the $conf model... >&2
  time -p cat `for a in \`seq 1 $num\`; do echo povidky_z_jedne_kapsy.nodia; done` | ../../src/korektor --input=untokenized_lines ../../models/korektor-czech-130202/$conf.conf >/dev/null
done
echo >&2
echo >&2
