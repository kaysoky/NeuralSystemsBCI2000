#ifndef ClassFilterH
#define ClassFilterH

#include "UGenericFilter.h"
#include <vector>

class ClassFilter : public GenericFilter
{
 friend class StatFilter;
 
 private:
       int samples;

      // int n_mat;                   // dimension of filter kernal = TransmitCh
      // int m_mat;                   // dimension of filter kernal = output channels

       std::vector<int> vc1,          // vertical chan #1
                        vf1,          // vertical freq #1
                        vc2,          // vertical chan #2
                        vf2,          // vertical freq #2

                        hc1,          // horizontal chan #1
                        hf1,          // horizontal freq #1
                        hc2,          // horizontal chan #2
                        hf2;          // horizontal freq #2

  //     float mat_ud[MAX_M][MAX_N];  // horizontal filter kernal matrix
  //     float mat_lr[MAX_M][MAX_N];  // vertical filter kernal matrix

       bool visualize;
       class GenericVisualization *vis;

       int n_hmat;                      // number of elements in horizontal function
       int class_mode;
       int n_vmat;                      // number of elements in vertical function
       float* feature[ 2 ],             //  values of the elements of the linear equations
            * wtmat[ 2 ];               //  weights of the elements of the linear equations

 public:
          ClassFilter();
  virtual ~ClassFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif


