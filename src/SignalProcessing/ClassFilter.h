//---------------------------------------------------------------------------

#ifndef ClassFilterH
#define ClassFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#define MAX_N  128
#define MAX_M  128

class ClassFilter : public GenericFilter
{
private:
       int      instance;
       int samples;

      // int n_mat;                   // dimension of filter kernal = TransmitCh
      // int m_mat;                   // dimension of filter kernal = output channels

       int vc1[MAX_M];                   // vertical chan #1
       int vf1[MAX_M];                   // vertical freq #1
       int vc2[MAX_M];                   // vertical chan #2
       int vf2[MAX_M];                   // vertical freq #2

       int hc1[MAX_M];                   // horizontal chan #1
       int hf1[MAX_M];                   // horizontal freq #1
       int hc2[MAX_M];                   // horizontal chan #2
       int hf2[MAX_M];                   // horizontal freq #2

  //     float mat_ud[MAX_M][MAX_N];  // horizontal filter kernal matrix
  //     float mat_lr[MAX_M][MAX_N];  // vertical filter kernal matrix

       bool visualize;
       GenericVisualization *vis;
public:
       int n_hmat;                      // number of elements in horizontal function
       int class_mode;
       int n_vmat;                      // number of elements in vertical function
       float feature[2][MAX_M];         //  values of the elements of the linear equations
       float wtmat[2][MAX_M];           //  weights of the elements of the linear equations

       ClassFilter(PARAMLIST *plist, STATELIST *slist);
       ClassFilter(PARAMLIST *plist, STATELIST *slist, int instance);
       ~ClassFilter();
       int Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *, int);
       int Process(GenericSignal *Input, GenericSignal *Output);
};
#endif


