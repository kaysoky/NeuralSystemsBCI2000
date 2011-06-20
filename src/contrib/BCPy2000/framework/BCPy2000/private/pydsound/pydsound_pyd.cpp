#include "pydsound_pyd.h"
#include "DX9Sound.h"

bool g_numpyInited = false;

void* NewSound(PyObject* arrayobj, double samplingFreq, unsigned short bitsPerSample)
{
	SoundRecord* p = new DX9Sound();
	SoundRecord::AddToGlobalList(p);
	if(arrayobj) {
#ifndef NO_IMPORT_ARRAY
		if(!g_numpyInited) {import_array1(NULL); g_numpyInited = true;}
#endif /* NO_IMPORT_ARRAY */
		PyArrayObject* array = (PyArrayObject*)arrayobj;
		// TODO:  assert that it (a) is an array (b) is 2-D (c) is double-precision floats
		p->Initialize((const double*)PyArray_DATA(array),
	                  (unsigned long)(0.5+samplingFreq), (unsigned long)bitsPerSample,
	                  PyArray_DIM(   array, 0), PyArray_DIM(   array, 1),
	                  PyArray_STRIDE(array, 0), PyArray_STRIDE(array, 1));
	}
	return (void*)p;
}

int  DeleteSound(void* id)
{
	SoundRecord* p = (SoundRecord*)id;
	if(!SoundRecord::IsListed(p)) return -1;
	SoundRecord::RemoveFromGlobalList(p);
	delete p;
	return 0;
}

int PlaySound(void* id, int nrepeats)
{
	SoundRecord* p = (SoundRecord*)id;
	if(!SoundRecord::IsListed(p)) return -1;
	if(!p->Play(nrepeats)) return -2;
	return 0;
}

int Check(void* id, double msecToSleepFirst)
{
	SoundRecord* p = (SoundRecord*)id;
	if(!SoundRecord::IsListed(p)) return 0;
	return p->Check(msecToSleepFirst);
}

int StopSound(void* id)
{
	SoundRecord* p = (SoundRecord*)id;
	if(!SoundRecord::IsListed(p)) return -1;
	if(!p->Stop()) return -2;
	return 0;
}

int PreplayLPT(void* id, int val, int port)
{
	SoundRecord* p = (SoundRecord*)id;
	if(!SoundRecord::IsListed(p)) return -1;
	p->InitPreplayLPT(val, port);
	return 0;
}

int PostplayLPT(void* id, int val, int port)
{
	SoundRecord* p = (SoundRecord*)id;
	if(!SoundRecord::IsListed(p)) return -1;
	p->InitPostplayLPT(val, port);
	return 0;
}

int SetSpeed(void* id, double val)
{
	SoundRecord* p = (SoundRecord*)id;
	if(!SoundRecord::IsListed(p)) return -1;
	if(!p->SetSpeed(val)) return -2;
	return 0;
}

int SetVolume(void* id, double val)
{
	SoundRecord* p = (SoundRecord*)id;
	if(!SoundRecord::IsListed(p)) return -1;
	if(!p->SetVolume(val)) return -2;
	return 0;
}

int SetPan(void* id, double val)
{
	SoundRecord* p = (SoundRecord*)id;
	if(!SoundRecord::IsListed(p)) return -1;
	if(!p->SetPan(val)) return -2;
	return 0;
}

void Cleanup(void)
{
	SoundRecord::DeleteAllInGlobalList();
}
/////////////////////////////////////////////////////////////////////////
