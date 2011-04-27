////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class that implements functionality common to all
//              file writer classes that output into a file.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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

class FileWriterBase: public GenericFileWriter
{
 protected:
          FileWriterBase( GenericOutputFormat& );
 public:
  virtual ~FileWriterBase();
  virtual void Publish() const;
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Write( const GenericSignal& Signal,
                      const StateVector&   Statevector );

	std::ofstream& File(){return mOutputFile;}
	GenericOutputFormat &OutputFormat(){return mrOutputFormat;}
	void WriteError();
 private:
  GenericOutputFormat&     mrOutputFormat;
  std::string              mFileName;
  std::ofstream            mOutputFile;
	
	std::queue<GenericSignal> mSignalQueue;
	std::queue<StateVector> mSVQueue;
	OSMutex mMutex;
	HANDLE mEventWrite;	

	class Writer : public OSThread
	{
	public:
		Writer(FileWriterBase *parent);
		~Writer();
		int Execute();
		void finish(){mFinish = true;}

	private:
		FileWriterBase *mParent;
		bool mFinish;
	};
	Writer *mWriter;
	friend class Writer;

};

#endif // FILE_WRITER_BASE_H
