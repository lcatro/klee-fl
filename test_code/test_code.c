

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char* get_message(const char* message) {
	if (strlen(message) <= 4)
		return NULL;

	if (!('F' == message[0] && '\x1' == message[1]))
		return NULL;

	unsigned int message_length = *(unsigned int*)&message[2];
	char* output_message = (char*)malloc(message_length);

	memcpy(output_message,&message[2],message_length);

	return output_message;
}

