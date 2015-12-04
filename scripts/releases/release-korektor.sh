#!/bin/bash

set -e

[ -n "$1" ] || { echo Usage: $0 korektor_version >&2; exit 1; }
korektor="$1"

dir=korektor-"$korektor"-bin
[ -d "$dir" ] && { echo Release $dir already exists >&2; exit 1; }

mkdir "$dir"

EXES=(korektor tokenizer create_error_model create_lm_binary create_morphology)
make -C ../../src clean
for b in linux32:linux-gcc-32.sh linux64:linux-gcc-64.sh win32:visual-cpp-32.sh win64:visual-cpp-64.sh osx:remote-osx-clang-32+64.sh\ manfred; do
  name="${b%%:*}"
  make="/net/projects/cpp_builtem/compilers/make-${b#*:}"

  # Choose make and targets
  case $name in
    linux*) targets=(${EXES[@]});;
    win*)   targets=(${EXES[@]/%/.exe});;
    osx*)   targets=(${EXES[@]});;
  esac

  # Compile Parsito
  mkdir "$dir/bin-$name"
  (cd ../../src && $make -j3 MODE=release ${targets[@]})

  # Copy files
  cp ${targets[@]/#/../../src/} "$dir/bin-$name"
done

make -C ../../src clean
rsync -a --exclude .gitignore ../../src/ "$dir/src"

make -C ../../documentation install
cp ../../INSTALL ../../MANUAL* ../../README ../../AUTHORS ../../LICENSE ../../CHANGES "$dir"
make -C ../../documentation manual.pdf
cp ../../documentation/manual.pdf "$dir"/MANUAL.pdf
make -C ../../documentation clean

echo All done
