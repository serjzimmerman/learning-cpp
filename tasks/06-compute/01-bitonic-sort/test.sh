# Set up variables
current_folder=${2:-./}
base_folder="resources"
passed=true

# ASCII colors
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

cd $current_folder/$base_folder

for file in *.dat; do
  echo -n "Testing $green$file$reset ..."
  $1 < $file > ans.tmp
  filename="${file}.ans"

  if diff -Z $filename ans.tmp; then
    echo "${green}Passed${reset}"
  else
    echo "${red}Failed${reset}"
    passed=false
  fi
done

if ${passed}
then
  exit 0
else
  exit 666
fi