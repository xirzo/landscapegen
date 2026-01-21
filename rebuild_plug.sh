#!/usr/bin/bash

libraries=("raylib")
lib_name="plug"

red='\e[0;31m'
green='\e[0;32m'
no_color='\e[0m'

read -p "Enter path to lib source file:" source_file

echo
if [[ -z ${CC} ]]; then
    echo -e "${red}C Compiler env (CC) is not set, aborting...${no_color}"
    exit 1
else
    echo -e "${green}Found C Compiler: \"${CC}\"${no_color}"
fi

link_libraries=""

for lib in $libraries; do
    ldconfig -p | grep ${lib}

    if [[ $? -ne 0 ]]; then
        echo -e "${red}Build failed, error: could not find ${lib}${no_color}"
        exit 1
    fi

    link_libraries+="-l${lib} "
done

echo
echo "Cleaning old files..."

rm ${lib_name}.so

echo
echo  "Current working directory:"
pwd
echo

echo "Building sourcefiles..."

$CC -shared -fPIC -o ${lib_name}.so ${source_file} ${link_libraries}

if [[ $? -ne  0 ]]; then
    echo -e "${red}Build failed, error code: ${?}${no_color}"
    exit 1
else
    echo -e "${green}Build ended successfully${no_color}"
fi

exit 0
