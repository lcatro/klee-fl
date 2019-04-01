
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test_code.h"


int main(int argc,char** argv) {
	if (2 == argc) {
		FILE* file = fopen(argv[1],"r");
		unsigned int file_length = 10;

		fseek(file,SEEK_END,0);

		file_length = ftell(file);

		fseek(file,SEEK_SET,0);

		char* file_buffer = (char*)malloc(file_length);

		fread(file_buffer,1,file_length,file);
		fclose(file);

		printf("file buffer length : %d \n",file_length);

		char* message_data = get_message(file_buffer);

		if (NULL == message_data)
			printf("Error for resolve message\n");
		else
			printf("message data : %s\n",message_data);
	}

	return 0;
}

