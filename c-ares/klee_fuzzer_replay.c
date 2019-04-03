
#include <ares.h>
#include <nameser.h>


int main(int argc,char** argv) {
  unsigned char *output_buffer;
  int output_buflen;
  unsigned char* input_buffer = "\x5c\x2e\x00\x5c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

  ares_create_query(input_buffer, ns_c_in, ns_t_a, 0x1234, 0, &output_buffer, &output_buflen, 0);

  return 0;
}
