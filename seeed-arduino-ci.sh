#!/bin/bash
set -aux

function _ci_usage() {
	cat <<EOF
Used in (Travis) CI script to build/test arduino project examples.

Syntax: $0 [ -b boards-spec1 -b boards-spec2 -b ... ] [ -s ] example-spec [ depend-library1 depend-library2 ... ]

        boards-spec(s): subject to Originze:platform:board, such as "arduino:avr:uno"

	-s            : only build examples specified in example-spec,
	                or else build all the found examples (if without -s option).

        example-spec:   subject to example1:prevent-boards/example2:prevent-boards/...
                        example(s) must exist in the project examples folder
                        if no prevents-boards, ':' could be discard.
                        prevent-boards must exist in boards-spec(s), specify the boards not build, 
                                       multiple boards could seperated by ','

        depend-library(s):
                        specify the depend libraries for this project, such as 'Seeed-Studio/Seeed_Arduino_FreeRTOS.git'
                        only support library repository located at https://github.com.

EOF
	exit 2
}

if [ "$#" == "0" ]; then
	echo "You have to enter at least one parameter, anything." >&2
	_ci_usage
fi

bd_ctr=0
find_examples=y
while getopts "b:sh" opt; do
	case "$opt" in
	b)	boards[$bd_ctr]="$OPTARG"
		bd_ctr=$(( bd_ctr + 1 ))
		;;

	s)	find_examples=
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

# : <<\__EOF__
if ! which arduino-cli; then
	if [ ! -f "$HOME/bin/arduino-cli" ]; then
		mkdir -p "$HOME/bin"
		# wget https://files.seeedstudio.com/arduino/arduino-cli_linux_64bit.tar.gz
		# tar xf arduino-cli_linux_64bit.tar.gz
		# rm arduino-cli_linux_64bit.tar.gz
		# mv arduino-cli $HOME/bin/
		curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR="$HOME/bin" sh
	fi
	export PATH="$PATH:$HOME/bin"
fi

arduino-cli core update-index --additional-urls https://downloads.arduino.cc/packages/package_index.json
arduino-cli core update-index --additional-urls http://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
arduino-cli core install arduino:avr    --additional-urls https://downloads.arduino.cc/packages/package_index.json
arduino-cli core install Seeeduino:samd --additional-urls http://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json

mkdir -p "$HOME/Arduino/libraries"
ln -s "$PWD" "$HOME/Arduino/libraries/."
# __EOF__

# parse the example-spec, such as
# readAngle:seeed_XIAO_m0,uno/fullFunction:seeed_XIAO_m0/resultoutput:seeed_XIAO_m0/fullFunction:uno/resultoutput:uno
eval exam_spec="\$$OPTIND"
exam_spec=( $(echo "$exam_spec" | tr '/' ' ') )
# echo exam_spec=${exam_spec[@]}

# the bash MAP make the example names uniq.
declare -A exam_map
declare -a examples
unbuild=" "
for (( i = 0; i < ${#exam_spec[@]}; i++ )); do
	k=${exam_spec[i]%%:*}
	v=${exam_spec[i]#*:}
	exam_map[$k]="$v"

	# no ':'
	if [ "$k" == "$v" ]; then
		continue
	fi

	exam_bds=( $(echo $v | tr ',' ' ') )
	for (( vi = 0; vi < ${#exam_bds[@]}; vi++ )); do
		unbuild="$unbuild$k:${exam_bds[vi]} "
	done
done
examples=${!exam_map[@]}
# echo unbuild=$unbuild

# shift out all argument begin with '-'
for (( i = 0; i < $OPTIND; i++)); do
	shift
done
# Get all the arduino dependent software
for arg in "$@"; do
	libname=$(echo $arg | cut -d "/" -f 2)
	if [[ -d "$HOME/Arduino/libraries/$libname" ]]; then
		ls $HOME/Arduino/libraries/$libname
	else
		git clone https://github.com/$arg $HOME/Arduino/libraries/$libname
		true
	fi
done

if [ "X$find_examples" != "X" ]; then
	unset examples

	exam_cnt=0
	exam_found=$(if test -d examples; then cd examples; find -type f -iname "*\.ino"; fi)
	for i in $exam_found; do
		v=${i%/*.ino}
		examples[$exam_cnt]=${v#./}
		(( exam_cnt++ ))
	done
fi
echo examples=${examples[@]}

#: <<\__EOF__
for i in ${examples[@]}; do
	example=$( echo $i | tr '@' '/' )
	for board in "${boards[@]}"; do
		pair=${example}:${board##*:}
		v=$(expr "$unbuild" : ".* $pair .*")
		[ $v -ne 0 ] && continue

		echo "Build ########## $example ########## on $board  ##########"
		arduino-cli compile  --warnings all --fqbn $board $PWD/examples/${example} --verbose
		r=$?
		[ $r -eq 0 ] && {
			echo "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
			echo "%%%OK%%%"
			echo "       %%%%%%%%%% $example %%%%%%%%%% on $board"
		} || {
			echo "%%%ERR%%%"
			echo "       %%%%%%%%%% $example %%%%%%%%%% on $board"
			exit 1
		}
	done
done
# __EOF__

exit 0
