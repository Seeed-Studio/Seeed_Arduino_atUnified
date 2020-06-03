#!/bin/bash
set -aux

function _ci_usage() {
	cat <<EOF
Used in (Travis) CI script to build/test arduino project examples.

Syntax: $0 [ -b boards-spec1 -b boards-spec2 -b ... ] example-spec [ depend-library1 depend-library2 ... ]

        boards-spec(s): subject to Originze:platform:board, such as "arduino:avr:uno"

        example-spec:   subject to example1:prevent-board/example2:prevent-board/...
                        example(s) must exist in the project examples folder
                        prevent-board must exist in boards-spec(s), specify the boards not build

        depend-library(s):
        	        specify the depend libraries for this project, such as Seeed-Studio/Seeed_Arduino_FreeRTOS.git
        	        only support library repository located at https://github.com.

EOF
	exit 2
}

if [ "$#" == "0" ]; then
	echo "You have to enter at least one parameter, anything." >&2
	_ci_usage
fi

bd_ctr=0
while getopts "b:h" opt; do
	case "$opt" in
	b)	boards[$bd_ctr]="$OPTARG"
		bd_ctr=$(( bd_ctr + 1 ))
		;;
	h)	_ci_usage;;
	esac
done

if (( $bd_ctr == 0 )); then
	boards[0]="arduino:avr:uno"
	boards[1]="Seeeduino:samd:seeed_XIAO_m0"
	boards[2]="Seeeduino:samd:seeed_wio_terminal"
	bd_ctr=3
fi

echo BOARDS="${boards[@]}"

if [ ! -f /usr/bin/arduino-cli  ] 
then
    mkdir -p "$HOME/bin"
    wget https://files.seeedstudio.com/arduino/arduino-cli_linux_64bit.tar.gz
    tar xf arduino-cli_linux_64bit.tar.gz
    rm arduino-cli_linux_64bit.tar.gz
    mv arduino-cli  $HOME/bin/
    export PATH="$PATH:$HOME/bin"
fi

arduino-cli core update-index --additional-urls http://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json

arduino-cli core install Seeeduino:samd --additional-urls http://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
arduino-cli core install arduino:avr


mkdir -p "$HOME/Arduino/libraries"
ln -s "$PWD" "$HOME/Arduino/libraries/."

unbuild[0]=""

counter=1
for arg; do 
    #arg[1] = readAngle:seeed_XIAO_m0/fullFunction:seeed_XIAO_m0/resultoutput:seeed_XIAO_m0/readAngle:uno/fullFunction:uno/resultoutput:uno
    if [ $counter == "1" ]
    then
        #Separate strings with "/"
        slash_num=$(echo $arg | awk -F"/" '{print NF-1}')
        for (( d=1; d<(slash_num+2); d++ ))
        do
            echo $arg |  cut -d "/" -f $d

            #storage substring into unbuild arrag
            unbuild[$d]=$(echo $arg |  cut -d "/" -f $d )
        done
    else
        echo $arg
        #Get all the arduino dependent software
        if [[ -d "$HOME/Arduino/libraries/$(echo $arg |  cut -d "/" -f 2)" ]]
        then
            ls $HOME/Arduino/libraries/$(echo $arg |  cut -d "/" -f 2)
        else
            git clone https://github.com/$arg  $HOME/Arduino/libraries/$(echo $arg |  cut -d "/" -f 2)
        fi
    fi
    counter=$((counter+1))
done


is_skip="false"

for board in "${boards[@]}"
do 
    #list all the libraries examples
    for example in examples/*
    do 
        for ub in "${unbuild[@]}"
        do
            #check if the example in unbuild 
            if [ "${example:9}:$(echo $board | cut -d ":" -f 3)" == "$ub" ]
            then
                is_skip="true"
                echo "skip...................$ub"
            fi
        done
        if [ "${is_skip}" == "false" ]
        then
            echo ${example:9} $board
            arduino-cli compile  --warnings all --fqbn $board $PWD/examples/${example:9}  --verbose; 
            if [ $? -eq 0 ]
            then
                echo "Success !!!!"
            else
                echo "Failure: I did not found IP address in file. Script failed" >&2
                exit 1
            fi
        fi
        is_skip="false"
    done
done
