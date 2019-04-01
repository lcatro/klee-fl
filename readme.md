

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

  5.编译klee-fuzzer 成BitCode

```sh

klee-build -bc ./test_fuzzing_entry.c

```

  6.编译klee 静态执行库

```sh

klee-build -bf .

```

  7.执行fuzzer 

```sh

sh run_fuzz.sh klee_fuzzer.bca

```

  8.输出结果

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

  9.回放Fuzzing 样本测试

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


