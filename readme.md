

## klee-fl

  KLEE-fl ,编译项目输出文件到Bitcode ,然后合并自定义Fuzzer 到一个整体的Bitcode 库引入到KLEE 进行符号执行.


## klee-fl 命令

  1.编译klee-fl .(TIPS:build_kleefl.sh 会安装编译工具到`/usr/local/bin` 目录)

```sh

sh ./build_kleefl.sh

```

  2.进入test_code 目录

```sh

cd test_code/
vim test_fuzzing_entry.c

```

  3.编写Fuzzer

```c

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "klee/klee.h"

#include "test_code.h"


int KleeFuzzingEntry(int argc,char** argv) {
  char buffer[0x10];

  printf("sizeof buffer = %lu\n",sizeof buffer);

  klee_make_symbolic(&buffer, sizeof buffer, "buffer");

  return (int)get_message(buffer);
}

```

  4.编译test_code

```sh

./configure CC=klee-clang
make

```

  5.编译klee-fuzzer 成BitCode 并合并成klee 静态执行库

```sh

klee-build ./test_fuzzing_entry.c .

```

  klee-build 格式如下:

```sh

Using : [ KFL_CFLAG="-I." ] klee-build %Fuzzer_Path% %Project_Build_Dir% [ import_file1 import_file2 .. ]
  Fuzzer_Path : Custom Fuzzer Path
  Project_Build_Dir : Project Path
  import_file : Import Link File Path
  KFL_CFLAG : Custom Compiler Flags
Example : 
  ./klee-build ./test_code/test_fuzzing_entry.c ./test_code/
  klee-build klee_fuzzer.c . libcares_la-ares_create_query.bc libcares_la-ares_library_init.bc

```

  6.执行fuzzer 

```sh

sh run_fuzz.sh klee_fuzzer.bca

```

  7.输出结果

```sh

MacBook-Pro-2:test_code root$ sh run_fuzz.sh klee_fuzzer.bca 
Output Klee Execute Command :
klee -libc=klee -entry-point KleeFuzzingEntry klee_fuzzer.bca
KLEE: output directory is "/Users/fc/Desktop/code_file/klee-fl/test_code/klee-out-0"
KLEE: Using Z3 solver backend
KLEE: WARNING: undefined reference to function: _fopen
KLEE: WARNING: undefined reference to function: __memcpy_chk
KLEE: WARNING: undefined reference to function: fclose
KLEE: WARNING: undefined reference to function: fread
KLEE: WARNING: undefined reference to function: fseek
KLEE: WARNING: undefined reference to function: ftell
KLEE: WARNING: undefined reference to function: printf
KLEE: WARNING: undefined reference to function: puts
KLEE: WARNING ONCE: calling external: printf(4687755904, 16) at test_fuzzing_entry.c:17 22
KLEE: ERROR: test_code.c:17: concretized symbolic size
KLEE: NOTE: now ignoring this error at this location
KLEE: WARNING ONCE: Alignment of memory from call "malloc" is not modelled. Using alignment of 8.
KLEE: WARNING ONCE: calling external: __memcpy_chk(4696031232, 4681917186, (ZExt w64 (ReadLSB w32 2 buffer)), 18446744073709551615) at /Users/fc/Desktop/code_file/klee-fl/test_code/test_code.c:9 0
KLEE: NOTE: found huge malloc, returning 0
KLEE: WARNING ONCE: Large alloc: 33554431 bytes.  KLEE may run out of memory.
KLEE: ERROR: test_code.c:19: failed external call: __memcpy_chk
KLEE: NOTE: now ignoring this error at this location
KLEE: ERROR: test_code.c:19: external call with symbolic argument: __memcpy_chk
KLEE: NOTE: now ignoring this error at this location
KLEE: NOTE: found huge malloc, returning 0
KLEE: NOTE: found huge malloc, returning 0
KLEE: NOTE: found huge malloc, returning 0
KLEE: NOTE: found huge malloc, returning 0
KLEE: NOTE: found huge malloc, returning 0
KLEE: NOTE: found huge malloc, returning 0
KLEE: NOTE: found huge malloc, returning 0
KLEE: ERROR: /Users/fc/Desktop/code_file/klee/runtime/klee-libc/strlen.c:14: memory error: out of bound pointer
KLEE: NOTE: now ignoring this error at this location
KLEE: NOTE: found huge malloc, returning 0
KLEE: NOTE: found huge malloc, returning 0

KLEE: done: total instructions = 563
KLEE: done: completed paths = 60
KLEE: done: generated tests = 32
sizeof buffer = 16
MacBook-Pro-2:test_code root$ 

```

  8.回放Fuzzing 样本测试

```sh

MacBook-Pro-2:test_code root$ ktest-tool klee-out-0/test000008.ktest 
ktest file : 'klee-out-0/test000008.ktest'
args       : ['klee_fuzzer.bca']
num objects: 1
object 0: name: 'buffer'
object 0: size: 16
object 0: data: b'F\x01\xff\xff\xff\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
object 0: hex : 0x4601ffffff00ffffffffffffffffffff
object 0: text: F...............
MacBook-Pro-2:test_code root$ 

```

## klee-fl 使用实例

  本例子以c-ares 库CVE-2016-5180 为例子.输入命令如下:

```sh

cd c-ares
./buildconf
./configure
make CC=klee-clang
klee-build klee_fuzzer.c . libcares_la-ares_create_query.bc libcares_la-ares_library_init.bc
sh run_fuzz.sh klee_fuzzer.bca

```

  执行`run_fuzz.sh` 脚本,也就意味着开始调用klee 执行编译出来的.bca 库文件.klee-fl 整体原理是参考libFuzzer 针对某一特定函数进行Fuzzer ,然后得出测试结果,klee 最后调用`KleeFuzzingEntry` 进入入口点,开始进行无输入Fuzzing 测试.因为有时候klee 在运行过程中耗时比较久,可以选择`Ctrl + C` 退出执行.进入klee 测试结果目录查看结果如下,我们关注.err 结尾的文件,这个就是崩溃测试样本:

```sh

MacBook-Pro-2:c-ares root$ cd klee-out-0
MacBook-Pro-2:klee-out-0 root$ ls | grep .err
test000003.ptr.err
MacBook-Pro-2:klee-out-0 root$

```

  使用`ktest-tool` 查看样本数据(这个数据是以.ktest 结尾的).

```sh

MacBook-Pro-2:klee-out-0 root$ cat 
test000003.ptr.err
MacBook-Pro-2:klee-out-0 root$ ktest-tool test000003.ktest 
ktest file : 'test000003.ktest'
args       : ['klee_fuzzer.bca']
num objects: 1
object 0: name: 'input_buffer'
object 0: size: 64
object 0: data: b'\\.\x00\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\'
object 0: hex : 0x5c2e005c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c5c
object 0: text: \..\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

```

  使用`cat` 查看崩溃信息(这个数据是以.err 结尾的).

```sh

MacBook-Pro-2:klee-out-0 root$ cat test000003.ptr.err 
Error: memory error: out of bound pointer
File: ares_create_query.c
Line: 196
assembly.ll line: 300
Stack: 
        #000000300 in ares_create_query (=4630147648, =1, =1, =4660, =0, =4624181984, =4624182016, =0) at ares_create_query.c:196
        #100000031 in KleeFuzzingEntry (=1, =4624534144) at klee_fuzzer.c:13
Info: 
        address: 4630188978
        next: object at 4624181064 of size 8
                MO8[8] allocated at KleeFuzzingEntry():  %3 = alloca i32, align 4
MacBook-Pro-2:klee-out-0 root$

```

  使用ASAN 复现该Bug .我们需要重新编译c-ares 库:

```sh

cd ..
./configure CFLAGS="-fsanitize=address -g"
make clean
clang klee_fuzzer_replay.c libcares_la-ares_create_query.o libcares_la-ares_library_init.o -fsanitize=address -g -o klee_fuzzer_replay
./klee_fuzzer_replay

```

  执行`./klee_fuzzer_test` ,输出结果如下:

```sh

MacBook-Pro-2:c-ares root$ ./klee_fuzzer_replay
=================================================================
==87944==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x6030000002c2 at pc 0x000104149764 bp 0x7ffeebab8690 sp 0x7ffeebab8688
WRITE of size 1 at 0x6030000002c2 thread T0
    #0 0x104149763 in ares_create_query ares_create_query.c:196
    #1 0x104148346 in main klee_fuzzer_replay.c:11
    #2 0x7fff790c1014 in start (libdyld.dylib:x86_64+0x1014)

0x6030000002c2 is located 0 bytes to the right of 18-byte region [0x6030000002b0,0x6030000002c2)
allocated by thread T0 here:
    #0 0x1041a8f53 in wrap_malloc (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x56f53)
    #1 0x10414889b in ares_create_query ares_create_query.c:133
    #2 0x104148346 in main klee_fuzzer_replay.c:11
    #3 0x7fff790c1014 in start (libdyld.dylib:x86_64+0x1014)

SUMMARY: AddressSanitizer: heap-buffer-overflow ares_create_query.c:196 in ares_create_query
Shadow bytes around the buggy address:
  0x1c0600000000: fa fa 00 00 00 fa fa fa 00 00 00 00 fa fa 00 00
  0x1c0600000010: 00 00 fa fa 00 00 00 00 fa fa fd fd fd fa fa fa
  0x1c0600000020: fd fd fd fd fa fa 00 00 00 fa fa fa 00 00 00 00
  0x1c0600000030: fa fa 00 00 00 fa fa fa 00 00 00 fa fa fa 00 00
  0x1c0600000040: 00 00 fa fa fd fd fd fa fa fa fd fd fd fd fa fa
=>0x1c0600000050: 00 00 00 00 fa fa 00 00[02]fa fa fa fa fa fa fa
  0x1c0600000060: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c0600000070: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c0600000080: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c0600000090: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c06000000a0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==87944==ABORTING
Abort trap: 6
MacBook-Pro-2:c-ares root$

```


## 内部参数

#### LLVM_COMPILER_PATH -- 配置LLVM 编译器路径

  LLVM_COMPILER_PATH 默认值为`/usr/bin/`.可指定LLVM 路径下的llvm-ar ,clang 和clang++ .用法:

```txt

LLVM_COMPILER_PATH=~/Desktop/code_file/llvm make

```

  或

```txt

export LLVM_COMPILER_PATH=~/Desktop/code_file/llvm
make

```

#### KFL_CLANG -- 配置clang 执行文件名

  KFL_CLANG 默认值为`clang`.用法:

```txt

KFL_CLANG=clang4 make

```

  或

```txt

export KFL_CLANG=clang4
make

```

#### KFL_CLANGXX -- 配置clang++ 执行文件名

  KFL_CLANGXX 默认值为`clang++`.用法:

```txt

KFL_CLANGXX=clang++4 make

```

  或

```txt

export KFL_CLANGXX=clang++4
make

```

#### KFL_LLVM_AR -- 配置llvm-ar 执行文件名

  KFL_LLVM_AR 默认值为`clang++`.用法:

```txt

KFL_LLVM_AR=./llvm3.3/llvm-ar make

```

  或

```txt

export KFL_LLVM_AR=./llvm3.3/llvm-ar
make

```


#### KFL_CFLAG -- 设置编译参数

  KFL_CFLAG 默认值为空.用法:

```txt

KFL_CFLAG="-g -I." make

```

  或

```txt

export KFL_CFLAG="-g -I."
make

```


