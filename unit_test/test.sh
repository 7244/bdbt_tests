#!/bin/bash

COMPILE_BASE_PARAM="-std=c++20 -Wall -Wextra"
COMPILE="clang++ $COMPILE_BASE_PARAM -oa.exe a.cpp"
declare -a g_COMPILE_SETTING=("" "-g" "-O0" "-O1" "-O2" "-O3" "-O3 -march=native -mtune=native")
declare -a g_BitOrderMatters=("-D__BitOrderMatters=false -D__TraverseFrom=2" "-D__BitOrderMatters=true -D__TraverseFrom=0" "-D__BitOrderMatters=true -D__TraverseFrom=1")

TIMEFORMAT=%R

function ex {
  echo executing $1
  time $1
  if [[ $? -ne 0 ]] ; then
    echo execute is failed, return value is not 0.
    exit 1
  fi
}

for COMPILE_SETTING in "${g_COMPILE_SETTING[@]}"
do
  for __bpn in 1 2 4 8
  do
    for BitOrderMatters in "${g_BitOrderMatters[@]}"
    do
      ex "$COMPILE $COMPILE_SETTING -D__bpn=$__bpn $BitOrderMatters"
      ex ./a.exe
    done
  done
done
