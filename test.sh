#!/bin/bash

: ${NB:=1000}
LC_ALL=C

die() {
    echo "$@" >&2
    exit 1
}

float_eval() {
    bash -c "enable -f $PWD/float_eval.so float_eval; float_eval '$@'; echo \$REPLY"
}

float_eval_bash () {
  local scale=2 res=

  if [[ $# > 0 ]]; then
      res=$(bc -q <<< "scale=$scale ; $*" 2> /dev/null)
  fi

  [[ "${res:0:1}" == "." ]] && res="0$res"

  REPLY="$res"
  [[ $res == 0 ]] && return 1
  return 0
}

TESTS=(
    "2"             "2"
    "1+2"           "3"
    "3*2"           "6"
    "1+2*3"         "7"
    "1*2+3"         "5"
    "2*(2+3)"       "10"
    "(2+3)*3"       "15"
    "(1+(2*3)+1)*2" "16"
    "(1+2)*(2+3)"   "15"
    "1*(2+3)*6+7"   "37"
    "1+(2+3)*6+7"   "38"
    "(1+2)*(2+3)+1" "16"
    "-2"            "-2"
    "-1+2"          "1"
    "1-2"           "-1"
    "-1-2"          "-3"
    "-(1+2)"        "-3"
    "-1-(1+2)"      "-4"
    )

for (( i = 0; i < ${#TESTS[@]}; i+=2 )); do
    res=$(float_eval "${TESTS[i]}")
    [[ "${res%%.000000}" == "${TESTS[i+1]}" ]] ||
        die "FAIL : '${TESTS[i]}' give '$res' instead of '${TESTS[i+1]}'"
done

echo "Everything OK"


echo -e "\n============= Benchmarks ===============\n"
echo "--> Builtins"
bash -c "enable -f $PWD/float_eval.so float_eval; time for (( i = 0; i < $NB; i++ )); do float_eval '1+2'; done"

echo -e "\n--> Bash (bc)"
time for (( i = 0; i < $NB; i++ )); do
    float_eval_bash "1+2"
done
