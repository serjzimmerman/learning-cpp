base_folder="resources"

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

current_folder=${2:-./}
for file in ${current_folder}/${base_folder}/*.dat; do
    echo "Generating ${green}${file}${reset} ... "

    # Check if an argument to executable location has been passed to the program
    if [ -z "$1" ]; then
        bin/ansgen < $file > ${file}.ans
    else
        $1 < $file > ${file}.ans
    fi
done