////////////////////////////////////////////////////////////////////////////////
// $Id: NeuralynxADC.h 2656 2010-08-03 dimitriadis $
// Author: g.dimitriadis@donders.ru.nl
// Description: A source class that interfaces to the Neuralynx DigitalLynx
//             amplifier through its reversed engineered
//              Matlab drivers
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef NEURALYNX_ADC_H
#define NEURALYNX_ADC_H


#include "GenericADC.h"
#include "NeuralynxThread.h"

class NeuralynxADC : public GenericADC
{
 public:
                    NeuralynxADC();
    virtual         ~NeuralynxADC();
    virtual void    Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void    Initialize( const SignalProperties&, const SignalProperties& );
    virtual void    Process( const GenericSignal&, GenericSignal& );
    virtual void    Halt();


 private:
    short samplingRate;
    void InitializeNeuralynxTread();//Start the Neuralynx data aquisition thread

protected:

    bool threadInitialized;
    bool keepNeuralynxThreadRunning;

    NeuralynxThread *dataThread;

};



#endif // NEURALYNX_ADC_H
