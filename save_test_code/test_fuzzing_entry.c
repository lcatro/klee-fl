

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
