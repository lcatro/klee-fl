
#include <ares.h>
#include <nameser.h>


int KleeFuzzingEntry(int argc,char** argv) {
  unsigned char *output_buffer;
  int output_buflen;
  unsigned char input_buffer[64] = {0};

  klee_make_symbolic(&input_buffer, sizeof(input_buffer), "input_buffer");

  ares_create_query(input_buffer, ns_c_in, ns_t_a, 0x1234, 0, &output_buffer, &output_buflen, 0);

  return 0;
}
