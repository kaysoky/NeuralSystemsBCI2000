#include"nrerror.h"

void nrerror(char error_text[])
{
	fprintf(stderr, "%s\n", error_text);
	exit(1);
}
