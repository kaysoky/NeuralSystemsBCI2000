////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class that implements functionality common to all
//              file writer classes that output into a file.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef FILE_WRITER_BASE_H
#define FILE_WRITER_BASE_H

#include "GenericFileWriter.h"
#include "GenericOutputFormat.h"

#include <string>
#include <fstream>
#include <queue>
#include "OSThread.h"
#include "OSMutex.h"
#include "OSEvent.h"

class FileWriterBase: public GenericFileWriter, OSThread
{
 protected:
          FileWriterBase( GenericOutputFormat& );
 public:
  virtual ~FileWriterBase();
  virtual void Publish();
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Halt();
  virtual void Write( const GenericSignal& Signal,
                      const StateVector&   Statevector );

 private:
  virtual int Execute();

  GenericOutputFormat&     mrOutputFormat;
  std::string              mFileName;
  std::ofstream            mOutputFile;

  std::queue<GenericSignal> mSignalQueue;
  std::queue<StateVector>   mStateVectorQueue;
  OSMutex mMutex;
  OSEvent mEvent;
};

#endif // FILE_WRITER_BASE_H
