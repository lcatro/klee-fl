

CC=clang
CXX=clang++


echo `$CC  -O2 -g klee-clang.c -o klee-clang`
echo `$CXX -O2 -g klee-build.cpp -o klee-build`

#rm -rf *.dSYM

cp ./klee-clang  /usr/local/bin/
cp ./klee-clang  /usr/local/bin/klee-clang++
cp ./klee-build  /usr/local/bin/
cp ./run_fuzz.sh /usr/local/bin/
