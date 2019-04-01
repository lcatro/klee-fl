
#!/bin/sh

build_file_path=$1

if [ -z "$build_file_path" ]; then
	echo Using : ./build_entry.sh %KleeFuzzingEntry_file_path%

	exit 1
fi


output_path=${build_file_path%.c*}
output_path=`echo "$output_path.bc"`

CC=clang
CXX=clang++

echo Output Compiler Command :
echo $CC -O2 -g $build_file_path -o $output_path
echo `$CC -O2 -g $build_file_path -o $output_path`

