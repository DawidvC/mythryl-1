#!/bin/ksh
#

ROOT=/home/sml/Dev/jhr/109.x

$ROOT/src/runtime/chunks/runtime7 --runtime-heap-image-to-run=$ROOT/heap/sml-cm <<XXXX
  make7::make ();
  basic_win.doit' (["/"], ":0.0", 20);
XXXX

