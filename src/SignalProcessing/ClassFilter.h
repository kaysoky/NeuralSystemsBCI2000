#ifndef ClassFilterH
#define ClassFilterH

#include "UGenericFilter.h"

class ClassFilter : public GenericFilter
{
 friend class StatFilter;
 
 private:
       enum
       {
#undef MAX_N
         MAX_N = 128,
#undef MAX_M
         MAX_M = 128,
       };
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
       class GenericVisualization *vis;

       int n_hmat;                      // number of elements in horizontal function
       int class_mode;
       int n_vmat;                      // number of elements in vertical function
       float feature[2][MAX_M];         //  values of the elements of the linear equations
       float wtmat[2][MAX_M];           //  weights of the elements of the linear equations

 public:
          ClassFilter();
  virtual ~ClassFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif


