//////////////////////////////////////////////////////////////////////////////////////
//
// File:        WindowingFunction.h
//
// Author:      juergen.mellinger@uni-tuebingen.de
//
// Description: A class that encapsulates details about windowing functions
//              used for sidelobe suppression in spectral analysis.
//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef WindowingFunctionH
#define WindowingFunctionH

class WindowingFunction
{
  // Class-related information.
  public:
    typedef float NumType;
    typedef enum
    {
      None = 0,
      Hamming,
      Hann,
      Blackman,

      NumWindows
    } Window;
    static const char* WindowNames( int i );

  private:
    static const struct WindowProperties
    {
      Window      mWindow;
      const char* mName;
      NumType     mGenerationCoeffs[ 2 ];
    } sWindowProperties[];

  // Instance-related information.
  public:
    WindowingFunction();
    WindowingFunction( int i );
    const char* Name() const;
    NumType Value( NumType ) const; // The argument specifies the position in the window.
                                    // It must be in the [0.0 ... 1.0[ range.
  private:
    Window mWindow;
};

#endif // WindowingFunctionH
