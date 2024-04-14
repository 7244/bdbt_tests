#!/bin/bash

CompilerBaseArg="-std=c++20 -Wall -Wextra -Wno-unused-function"
CompilerBaseArg+=" a.cpp"

declare -a g_Compiler=("clang++" "g++")
declare -a g_CompileSetting=("" "-g" "-O0" "-O1" "-O2" "-O3" "-O3 -march=native -mtune=native")
declare -a g_BitOrderMatters=("-D__BitOrderMatters=false -D__TraverseFrom=2" "-D__BitOrderMatters=true -D__TraverseFrom=0" "-D__BitOrderMatters=true -D__TraverseFrom=1")
declare -a g_KeySize=("" "-D__KeySize=0")

TIMEFORMAT=%R

function ex {
  echo executing $1
  time $1
  if [[ $? -ne 0 ]] ; then
    echo execute is failed, return value is not 0.
    exit 1
  fi
}

for CompileSetting in "${g_CompileSetting[@]}"
do
  for __bpn in 1 2 4 8
  do
    for BitOrderMatters in "${g_BitOrderMatters[@]}"
    do
      for RealKeySize in 8 16 24 32 40 48 56 64
      do
        for KeySize in "${g_KeySize[@]}"
        do
          for Compiler in "${g_Compiler[@]}"
          do
            ex "$Compiler $CompilerBaseArg -oa.exe $CompileSetting -DRealKeySize=$RealKeySize -D__bpn=$__bpn $BitOrderMatters $KeySize"
            ex ./a.exe
          done
        done
      done
    done
  done
done
