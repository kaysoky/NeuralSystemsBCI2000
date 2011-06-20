#ifdef __cplusplus
extern "C" {
#endif

#include "Python.h"
#include "numpy/arrayobject.h"

void* NewSound(PyObject* array, double samplingFreq, unsigned short bitsPerSample);
int  DeleteSound(void* id);
int  PlaySound(void* id, int nrepeats);
int  Check(void* id, double msecToSleepFirst);
int  StopSound(void* id);

int  PreplayLPT(void* id, int val, int port);
int  PostplayLPT(void* id, int val, int port);
int  SetSpeed(void* id, double val);
int  SetVolume(void* id, double val);
int  SetPan(void* id, double val);
void Cleanup(void);


#ifdef __cplusplus
}
#endif
