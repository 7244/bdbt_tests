#!/bin/bash

CompilerBaseArg="-std=c++20 -Wall -Wextra -Wno-unused-function"
CompilerBaseArg+=" a.cpp"

TIMEFORMAT=%R

function cleanup {
  rm /tmp/bdbt_test.exe 2> /dev/null
}
function exit2 {
  cleanup
  exit $1
}

function do_test {
  declare -a g_Compiler=("clang++" "g++")
  declare -a g_CompileSetting=("" "-g" "-O0" "-O1" "-O2" "-O3" "-O3 -march=native -mtune=native")
  declare -a g_BitOrderMatters=("-D__BitOrderMatters=false -D__TraverseFrom=2" "-D__BitOrderMatters=true -D__TraverseFrom=0" "-D__BitOrderMatters=true -D__TraverseFrom=1")
  declare -a g_KeySize=("" "-D__KeySize=0")

  function ex {
    echo executing $1
    $1 -o/tmp/bdbt_test.exe
    if [[ $? -ne 0 ]] ; then
      echo compile is failed, return value is not 0.
      exit2 1
    fi
    /tmp/bdbt_test.exe
    if [[ $? -ne 0 ]] ; then
      echo execute is failed, return value is not 0.
      exit2 1
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
              ex "$Compiler $CompilerBaseArg $CompileSetting -DRealKeySize=$RealKeySize -D__bpn=$__bpn $BitOrderMatters $KeySize"
            done
          done
        done
      done
    done
  done

  cleanup
}

function do_bench {
  declare -a g_Compiler=("clang++" "g++")
  declare -a g_CompileSetting=("-O3" "-O3 -march=native -mtune=native")
  declare -a g_BitOrderMatters=("-D__BitOrderMatters=false -D__TraverseFrom=2" "-D__BitOrderMatters=true -D__TraverseFrom=0" "-D__BitOrderMatters=true -D__TraverseFrom=1")
  declare -a g_KeySize=("" "-D__KeySize=0")

  function ex {
    $1
    if [[ $? -ne 0 ]] ; then
      echo execute is failed, return value is not 0.
      exit 1
    fi
  }
  function ex_time {
    time "./a.exe"
    if [[ $? -ne 0 ]] ; then
      echo execute is failed, return value is not 0.
      exit 1
    fi
  }

  sum0=0
  sum1=0

  for CompileSetting in "${g_CompileSetting[@]}"
  do
    for BitOrderMatters in "${g_BitOrderMatters[@]}"
    do
      for KeySize in "${g_KeySize[@]}"
      do
        for Compiler in "${g_Compiler[@]}"
        do
          ex "$Compiler $CompilerBaseArg -oa.exe $CompileSetting -DRealKeySize=32 -D__bpn=2 $BitOrderMatters $KeySize"
          output=$( { ex_time ./a.exe; } 2>&1 )
          if [[ $Compiler == "clang++" ]] ; then
            sum0=$(awk "BEGIN {print $sum0 + $output}")
          elif [[ $Compiler == "g++" ]] ; then
            sum1=$(awk "BEGIN {print $sum1 + $output}")
          fi
        done
      done
    done
  done

  echo $sum0
  echo $sum1
}

trap_int() {
  cleanup
}
trap trap_int INT

if [[ "$#" -ne 0 ]] ; then
  if [[ "$1" == "test" ]] ; then
    do_test
  elif [[ "$1" == "bench" ]] ; then
    do_bench
  fi
else
  do_test
fi
