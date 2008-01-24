/**
 * Program:   Biosemi2ADC
 * Module:    Biosemi2ADC.CPP
 * Comment:   Acquires from a Biosemi Act2, tested with a MK1 may work
 *      on a MK2
 * Version:   0.01
 * Liecense:
 * Copyright (C) 2005 Samuel A. Inverso (samuel.inverso@gmail.com), Yang Zhen
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 
 * USA
 *
 * Revisions:
 *  $Log$
 *  Revision 1.1  2005/12/12 00:05:24  sinverso
 *  Initial Revision: Working and tested offline. Not tested in real experiments.
 *
 */
#ifndef Biosemi2ADCH
#define Biosemi2ADCH

#include <vector>
#include <string>
#include "GenericADC.h"
#include "PrecisionTime.h"
#include "Biosemi2Client.h"

class Biosemi2ADC : public GenericADC
{
public:
                  Biosemi2ADC();
    virtual      ~Biosemi2ADC();

    virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void Initialize( const SignalProperties&, const SignalProperties& );
    virtual void Process( const GenericSignal&, GenericSignal& );
    virtual void Halt();

protected:
    static const int TRIGGER_HIGH = 1;
    static const int TRIGGER_LOW = 0;
    static const int BATTERY_LOW = true;
    static const int BATTERY_NOT_LOW = false;

    int _samplingRate;
    int _softwareCh;     // number of channels to send

    /**
     * The number of signal channels
     * If we are postfixing triggers than:
     *  _softwareCh = _signalChannels + NUM_TRIGGERS
     * else
     *  _softwareCh = _signalChannels
     */
    int _signalChannels;


    int _sampleBlockSize; // sample blocksize to send
    mutable Biosemi2Client _biosemi;

    Biosemi2Client::DataBlock *_dataBlock;
    std::vector<std::string> _triggerNames;
    bool _postfixTriggers; // if true, place the triggers after the
                            // the eeg channels
    int _triggerScaleMultiplier;

};

#endif // Biosemi2ADCH

