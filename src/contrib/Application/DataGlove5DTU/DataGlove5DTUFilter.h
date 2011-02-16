/* $BEGIN_BCI2000_LICENSE$
 * 
 * This file is part of BCI2000, a platform for real-time bio-signal research.
 * [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
 * 
 * BCI2000 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * BCI2000 is distributed in the hope that it will be useful, but
 *                         WITHOUT ANY WARRANTY
 * - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * $END_BCI2000_LICENSE$
/*/
#ifndef DataGlove5DTUFilterH
#define DataGlove5DTUFilterH

#include "GenericFilter.h"
#include "fglove.h"


typedef fdGlove*        (*FDOPEN)                   (char    *pPort);
typedef int             (*FDCLOSE)                  (fdGlove *pFG  );
typedef int             (*FDGETGLOVETYPE)           (fdGlove *pFG  );
typedef int             (*FDGETNUMSENSORS)          (fdGlove *pFG  );
typedef int             (*FDGETGLOVEHAND)           (fdGlove *pFG  );
typedef int             (*FDSCANUSB)                (unsigned short *aPID,  int &nNumMax);
typedef void            (*FDGETSENSORRAWALL)        (fdGlove *pFG,          unsigned short *pData);
typedef unsigned short  (*FDGETSENSORRAW)           (fdGlove *pFG,          int nSensor);


class DataGlove5DTUFilter : public GenericFilter
{
 public:
          DataGlove5DTUFilter();
  virtual ~DataGlove5DTUFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual bool AllowsVisualization() const { return false; }

 private:

  HINSTANCE                         hinstLib;
  FDOPEN                            fdOpenCall;
  FDCLOSE                           fdCloseCall;
  FDSCANUSB                         fdScanUSBCall;
  FDGETGLOVETYPE                    fdGetGloveTypeCall;
  FDGETGLOVEHAND                    fdGetGloveHandCall;
  FDGETNUMSENSORS                   fdGetNumSensorsCall;
  FDGETSENSORRAWALL                 fdGetSensorRawAllCall;
  FDGETSENSORRAW                    fdGetSensorRawCall;
	fdGlove                          *pGlove;
  int                               ret;
	char                             *szPort;
	char	                            szPortToOpen[6];
	int                               glovetype;
	int                               glovehand;
  int                               glovesensors;
  bool                              datagloveenable;

  unsigned short                    sensor_data[14];




};

#endif // DataGlove5DTUFilterH




