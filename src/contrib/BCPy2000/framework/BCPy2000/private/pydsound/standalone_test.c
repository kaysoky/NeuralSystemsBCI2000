#undef NO_IMPORT_ARRAY

#include "pydsound_pyd.h"
#include <stdio.h>

int main(int argc, const char *argv[])
{
	void* id;
	npy_intp dims[2] = {44100, 2};
	PyObject* a = NULL;
	
	if(1) {
#ifndef NO_IMPORT_ARRAY
		printf("initializing numpy...\n");
		import_array1(-1); // crashes
#endif /* NO_IMPORT_ARRAY */
		printf("creating array...\n");
		a = PyArray_SimpleNew(2, dims, PyArray_DOUBLE); //crashes without import_array (unfortunately import_array also crashes)
	}

	printf("a = 0x%08x\n", a);	
	id = NewSound(a, 44100.0, 16);
	PreplayLPT(id, 255, 2);
	PostplayLPT(id, 0, 2);
	PlaySound(id, 1);	
	PlaySound(((char*)id)+1, 1);
	Cleanup();
	printf("done\n");
	return 0;
}
