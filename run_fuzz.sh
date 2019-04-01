

KLEE="klee"
TARGET_FUZZER_BINARY=$1

#
#if [ -z "$TARGET_BINARY" ]; then
#	echo ARGUMENT ERROR : LOST TARGET_BINARY !..
#	echo Using : ./run_fuzz.sh %TARGET_BINARY% %FUZZER_PATH%
#
#	exit 1
#fi

if [ -z "$TARGET_FUZZER_BINARY" ]; then
	echo ARGUMENT ERROR : LOST TARGET_FUZZER_BINARY !..
	echo Using : run_fuzz.sh %TARGET_FUZZER_BINARY%

	exit 1
fi


echo Output Klee Execute Command :
echo $KLEE -libc=klee -entry-point KleeFuzzingEntry $TARGET_FUZZER_BINARY
echo `$KLEE -libc=klee -entry-point KleeFuzzingEntry $TARGET_FUZZER_BINARY`
