
#!/bin/sh


if [ $# -eq 0 ] || [ -z "$1" ] || [ -z "$2" ]; then
    echo "No arguments supplied"
	exit 1;
elif [ $# -eq 2 ]; then
	if [ "$1" != 'clean' ] && [ "$1" != 'clean_binary' ] && [ "$1" != 'compile' ] && [ "$1" != 'record' ] && [ "$1" != 'run' ]; then
		echo "First argument is invalid (Valid Commands: clean/clean_binary/compile/record/run)"
		exit 1
	fi
	if [ "$2" != 'debug' ] && [ "$2" != 'release' ]; then
		echo "Second argument is invalid (Valid Configurations: debug/release)"
		exit 1
	fi
else
	echo "Invalid amount of arguments passed (Arg1 = clean/clean_binary/compile/record/run, Arg2 = debug/release)"
	exit 1
fi




# Variables responsible for making the compile_commands.json and symlinking it later.
SCRIPT_PATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
SCRIPT_ABS_PATH="$( cygpath -w "${SCRIPT_PATH}" )"
COMMANDS_SYMLINK_PATH="${SCRIPT_PATH}/.."
COMPILE_DATABASE_FILENAME="compile_commands"
JSON=".json"
MAKEFILE_THREAD_JOBS="16"
COMPILE_DATABASE_ABSOLUTE_FILENAME_SYMLINK="${COMMANDS_SYMLINK_PATH}/${COMPILE_DATABASE_FILENAME}${JSON}"
COMPILE_DATABASE_ABSOLUTE_FILENAME="${SCRIPT_PATH}/${COMPILE_DATABASE_FILENAME}"


DEBUG_POSTFIX="_debug"
RELEASE_POSTFIX="_release"


# Make Command & finding the path of the internal makefile
MAKEFILE_NAME="makefile.mf"
MAKEFILE_RELATIVE_PATH=" $( realpath --relative-to="${SCRIPT_PATH}/../../" "${SCRIPT_PATH}/${MAKEFILE_NAME}") "
MAKE_COMMAND="make -f ${MAKEFILE_RELATIVE_PATH}"



if [ "$2" == 'release' ]; then
	export DEBUG=0
	MAKE_CONFIG='build'
	COMPILE_DATABASE_ABSOLUTE_FILENAME+="${RELEASE_POSTFIX}${JSON}"

elif [ "$2" == 'debug' ]; then
	export DEBUG=1
	MAKE_CONFIG='build'
	COMPILE_DATABASE_ABSOLUTE_FILENAME+="${DEBUG_POSTFIX}${JSON}"

fi




function clean_build() {
	${MAKE_COMMAND} clean_internal
}

function clean_binary() {
	${MAKE_COMMAND} cleanbin_internal
}

function compile() {
	${MAKE_COMMAND} ${MAKE_CONFIG}
}

function record_build() {
	# echo ${MAKE_COMMAND} ${MAKE_CONFIG}
	# echo ${SCRIPT_ABS_PATH}
	python "${SCRIPT_ABS_PATH}"/compile_comms.py --out="${COMPILE_DATABASE_ABSOLUTE_FILENAME}" --exec="${MAKE_COMMAND} -j ${MAKEFILE_THREAD_JOBS} ${MAKE_CONFIG} && ${MAKE_COMMAND} ${MAKE_CONFIG}"
	# python3 /cygdrive/c/"Program Files/Programming Utillities"/Cygwin${SCRIPT_PATH}/compile_commands.py --out=${COMPILE_DATABASE_ABSOLUTE_FILENAME} --exec=${tmp}
}

function run_build() {
	${MAKE_COMMAND} run
}

function create_symlink() {
	export CYGWIN=winsymlinks:nativestrict
	# create symlink to the currently/previously created compile_commands_(debug/release).json ( in ../ [.vscode/] )
	echo "ln -sf " "$COMPILE_DATABASE_ABSOLUTE_FILENAME" "$COMPILE_DATABASE_ABSOLUTE_FILENAME_SYMLINK"
	ln -sf "$COMPILE_DATABASE_ABSOLUTE_FILENAME" "$COMPILE_DATABASE_ABSOLUTE_FILENAME_SYMLINK"
}


if [ "$1" == 'clean' ]; then
	clean_build
elif [ "$1" == 'clean_binary' ]; then
	clean_binary
elif [ "$1" == 'compile' ]; then
	compile
	create_symlink
	# We need to bind the symlink to the current compile config 
	# to get correct references and debug info.
elif [ "$1" == 'run' ]; then
	run_build
elif [ "$1" == 'record' ]; then
	clean_build    # Clean the build that was chosen
	record_build   # Record build process using bear for the compile_commands.json 
	create_symlink # Create the symlink from bear output json file
	echo "compile_commands.json (symlink=${COMPILE_DATABASE_ABSOLUTE_FILENAME_SYMLINK}) created from ${COMPILE_DATABASE_ABSOLUTE_FILENAME}";
else
	echo "You shouldn't have reached this place. Something went horribly wrong."
	exit 1
fi

echo -n
echo -n "Shell Script Finished with args [Command Config] = [$1 $2]"
echo -n