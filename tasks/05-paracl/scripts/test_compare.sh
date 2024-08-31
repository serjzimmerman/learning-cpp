#!/bin/sh

current_folder=${2:-./}
passed=0

ansfile=$(mktemp /tmp/paracl-temp.tmp.XXXXXX)
binfile=$(mktemp /tmp/paracl-temp.tmp.XXXXXX)

for file in $current_folder/*.pcl; do
  echo -n "Testing ${green}${file}${reset} ... "
  if [ -f "${file}.in" ]; then
    $1 $file < ${file}.in > $ansfile
  else 
    $1 $file > $ansfile
  fi

  if diff -Z ${file}.ans $ansfile; then
    echo "${green}Passed${reset}"
  else
    echo "${red}Failed${reset}"
    passed=1
  fi

  $1 $file -o$binfile
  if [ -f "${file}.in" ]; then
    $3 $binfile < ${file}.in > $ansfile
  else 
    $3 $binfile > $ansfile
  fi

  if diff -Z ${file}.ans $ansfile; then
    echo "${green}Passed${reset}"
  else
    echo "${red}Failed${reset}"
    passed=1
  fi
done

exit $passed