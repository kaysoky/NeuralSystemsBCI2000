#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../src/message.h"

int main(void) {
	printf("sizeof(INT16_T) = %d\n", sizeof(INT16_T));
	printf("sizeof(INT32_T) = %d\n", sizeof(INT32_T));
	printf("sizeof(INT64_T) = %d\n", sizeof(INT64_T));
	printf("sizeof(int16_t) = %d\n", sizeof(int16_t));
	printf("sizeof(int32_t) = %d\n", sizeof(int32_t));
	printf("sizeof(int64_t) = %d\n", sizeof(int64_t));
}

