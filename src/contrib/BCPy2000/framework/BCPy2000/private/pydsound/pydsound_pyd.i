%module pydsound
%{
#include "pydsound_pyd.h"
%}

void* NewSound(PyObject* array, double samplingFreq=44100.0, unsigned short bitsPerSample=16);
int  DeleteSound(void* id);
int  PlaySound(void* id, int nrepeats=1);
int  Check(void* id, double msecToSleepFirst=0.0);
int  StopSound(void* id);

int  PreplayLPT( void* id, int val=-1, int port=0);
int  PostplayLPT(void* id, int val=-1, int port=0);
int  SetSpeed(void* id, double val);
int  SetVolume(void* id, double val);
int  SetPan(void* id, double val);
void Cleanup(void);

