

CC=clang
CXX=clang++


echo `$CXX -O2 -g klee-clang.cpp -o klee-clang`
echo `$CXX -O2 -g klee-build.cpp -o klee-build`

rm -rf *.dSYM

cp ./klee-clang    /usr/local/bin/
cp ./klee-clang   ./klee-clang++
cp ./klee-clang++  /usr/local/bin/
cp ./klee-build    /usr/local/bin/
cp ./run_fuzz.sh   /usr/local/bin/
