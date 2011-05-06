/////////////////////////////////////////////////////////////////////////////
// $Id: NIDAQLogger.cpp 2118 2008-09-04 15:36:18Z renga $                  //
// Author: justin.renga@gmail.com                                          //
// Description: An output filter for National Instruments Data Acquisition //
//              Boards                                                     //
//                                                                         //
// (C) 2000-2008, BCI2000 Project                                          //
// http://www.bci2000.org                                                  //
/////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop
#include "NIDAQFilter.h"
#include "BCIEvent.h"
#include "ExpressionFilter.h"
#include "NIDAQmx.h"
#include <sstream>
using namespace std;

RegisterFilter(NIDAQFilter,1.01);

// The default constructor (overloaded) //
NIDAQFilter::NIDAQFilter()
{
	// INITIALIZE THINGS //
	mRan = false;
	mAnalog = mDigital = NULL;
	mFound[0] = mFound[1] = 0;
	mCounter[0] = mCounter[1] = 0;
	mDevs[0] = mDevs[1] = "NULL";
	mActive[0] = mActive[1] = "";
	// BEGIN "PUBLISH" //
	if (OptionalParameter("LogNIDAQout") == 1)
	{
		// Collect Device Names //
		vector<string>		lDevices;
		char				lPValue[32];	// the full parameter value
		unsigned long int	lSerial = 0;	// the serial number of the device (to test DAQmx compatibility)
		string				lDevice;		// the device name
		string				lParam;			// the ports (and other paramters)
		string				lExpress = "Filtering:NIFilter matrix FilterExpressions= { ";	// the expression matrix string
		if ((lDevices = CollectDeviceNames()).empty()) return; // error message already dealt with
		if (OptionalParameter("LogDigiOut") != "")	// if the user wants to use digital output
		{
			// copy the parameter value into a char[] so that it can be tokenized
			strncpy(lPValue,((string)OptionalParameter("LogDigiOut")).c_str(),((string)OptionalParameter("LogDigiOut")).size()+1);
			mDevs[0]	=	strtok(lPValue,"-");	// grab the device name
			lParam	=	strtok(NULL,"-");			// grab the port specifications
			DAQmxGetDevSerialNum(lDevice.c_str(),&lSerial);	// determine if the device supports DAQmx 8.x
			if (lSerial == 0)
			{
				bcierr << mDevs[0] << " is either not connected properly or does not support DAQmx 8.x . Check connections and try again" << endl;
				return;
			}
			// Check to see if the device is connected -> If not, throw an error and quit //
			if (!find(lDevice,lDevices))
			{
				bcierr << mDevs[0] << " is not connected to the computer. Please check connections and try again" << endl;
				return;
			}
			// Get Number of physical ports on device //
			if ((mFound[0] = GetNumDigitalLines(mDevs[0])) < 0)
			{
				bcierr << "No digital lines were detected on " << mDevs[0] << endl;
				return;
			}
			// Check port length -> does it match number of physical ports? If not, throw an error and quit //
			if (lParam.size() != mFound[0])
			{
				bcierr	<< "Unequal lengths detected. Parameter length (" << lParam.size() << ") does not match number of physical channels (" <<
						mFound[0] << ")" << endl;
			}
			// Go through each item in the port list -> are all of the characters valid? If one isn't, throw an error and quit //
			for (int i = 0; i < (int)lParam.size(); i++)
			{
				int charVal = lParam.at(i)-48;
				if (charVal < 0 || charVal > 1)
				{
					bcierr << "Invalid character detected at port " << i << ". Valid characters and functions are listed below:\n" <<
						"0: Port is not being used.\n1: Port is being used for input purposes.\nPlease correct parameter list and try again" << endl;
					return;
				}
				mLines.push_back(charVal);	// push back the "activeness" of the port (0 -> false [inactive], 1 -> true [active])
				if (charVal)
				{
					if (!mActive[0].empty())
						mActive[0].append(", ");
					mActive[0].append(mLNames[i]);
					lExpress.append(mLNames[i]).append(" ");
				}
			}
		}
		if ((OptionalParameter("LogAnaOut") != ""))	// if the user wants to use analog output
		{
			// copy the parameter value into a char[] so that it is possible to tokenize it
			strncpy(lPValue,((string)OptionalParameter("LogAnaOut")).c_str(),((string)OptionalParameter("LogAnaOut")).size()+1);
			mDevs[1] = strtok(lPValue,"-");	// grab the device name
			lParam = strtok(NULL,"-");	// grab the port values
			DAQmxGetDevSerialNum(mDevs[1].c_str(),&lSerial);	// checks to see if this is device supports DAQmx 8.x drivers
			if (lSerial == 0)
			{
				bcierr << lDevice << " is either not connected properly or does not support DAQmx 8.x . Check connections and try again" << endl;
				return;
			}
			if (!find(mDevs[1],lDevices))	// is the device connected to the computer?
			{
				bcierr << mDevs[1] << " is not connected to the computer. Please check connections and try again" << endl;
				return;
			}
			if ((mFound[1] = GetNumAnalogOutputLines(mDevs[1])) < 0)	// try to get the number of analog output lines
			{
				bcierr << "No analog input lines were detected on " << mDevs[1] << endl;
				return;
			}
			if (lParam.size() != mFound[1])	// if the number of ports the user specified does not match the number of ports actually found...
			{
				bcierr	<< "Unequal lengths detected. Parameter length (" << lParam.size() << ") does not match number of physical channels (" <<
						mFound[1] << ")" << endl;
				return;
			}
			// Go through each item in the port list -> are all of the characters valid? If one isn't, throw an error and quit //
			for (int i = 0; i < (int)lParam.size(); i++)
			{
				int charVal = lParam.at(i)-48;
				if (charVal < 0 || charVal > 1)
				{
					bcierr << "Invalid character detected at port " << i << ". Valid characters and functions are listed below:\n" <<
						"0: Port is not being used.\n1: Port is being used for input purposes.\nPlease correct parameter list and try again" << endl;
					return;
				}
				mLines.push_back(charVal);	// push the value back into mLines (0 -> false, 1 -> true)
				if (charVal)	// if we encountered an active port...
				{
					if (!mActive[1].empty())	// add lines to mActive[1] (which will be used for the task handle
						mActive[1].append(", ");
					mActive[1].append(mLNames[i+mFound[0]]);
					lExpress.append(mLNames[i+mFound[0]]).append(" ");	// add the line name into lExpress (the string for making the expressions matrix)
				}
			}
			if (!AcquireAOVRanges()) return;		// error message already taken care of inside function
			if (!mRanges.empty() && mRanges.size() > 2)	// create the enumeration for the output voltages
			{
				string lParameter = "Filtering:NIFilter int ";
				lParameter.append(mDevs[1]);
				lParameter.append("OVRanges= 0 0 0 ");
				lParameter.append(IntToString((mRanges.size()/2)-1));
				lParameter.append(" // Support Output Voltage Ranges ");
				for (int i = 0; i < (int)mRanges.size(); i += 2)	// 1: 0.0<->5.0 , for example
				{
					lParameter.append(IntToString(i/2));
					lParameter.append(": ");
					lParameter.append(FloatToString(mRanges[i]));
					lParameter.append("<->");
					lParameter.append(FloatToString(mRanges[i+1]));
					lParameter.append(" ");
				}
				lParameter.append("(enumeration)");
				BEGIN_PARAMETER_DEFINITIONS
					lParameter.c_str(),
				END_PARAMETER_DEFINITIONS
			}
		}
		if (mFound[0] + mFound[1])
		{
			lExpress.append("} { Expressions } ");
			for (int i = 0; i < (int)mLines.size(); i++)
				lExpress.append("0 ");
			BEGIN_PARAMETER_DEFINITIONS		// construct the "FilterExpressions" parameter //
				lExpress.append("// expressions for digital/analog outputs (matrix)").c_str(),
			END_PARAMETER_DEFINITIONS
		}
		for (int i = 0; i < (int)mLines.size(); i++)	// Construct the states for active channels //
		{
			if (mLines[i])
			{
				string states = "NI";
				states.append((i < mFound[0] ? mDevs[0].c_str() : mDevs[1].c_str()));
				states.append((i < mFound[0] ? "DOUTPUT" : "AOUTPUT"));
				states.append(IntToString((i < mFound[0] ? i : i-mFound[0])));
				states.append((i < mFound[0] ? " 1 0 0 0" : " 16 0 0 0"));
				BEGIN_EVENT_DEFINITIONS
					states.c_str(),
				END_EVENT_DEFINITIONS
			}
		}
	}
	// END "PUBLISH" //
}
// The default destructor (overloaded) //
NIDAQFilter::~NIDAQFilter()
{
  if (mAnalog) // if mAnalog has been used before (it is not NULL), try to clear it
    if (ReportError(DAQmxClearTask(mAnalog)) < 0)
      bcierr << "Unable to clear analog task" << endl;
  if (mDigital)	// if mDigital has been used before (it is not NULL), try to clear it
    if (ReportError(DAQmxClearTask(mDigital)) < 0)
      bcierr << "Unable to clear digital task" << endl;
  // Clear the vectors //
  mLines.clear();
  mRanges.clear();
  mExpressions.clear();
  mLNames.clear();
}
// Make sure that everything is alright //
void
NIDAQFilter::Preflight(const SignalProperties& Input, SignalProperties& Output) const
{
	Output = Input;
	if (OptionalParameter("LogNIDAQout") > 0) // is the filter being used?
	{
		if (!mLines.empty()) // is mLines empty()?
		{
			int localCounter = 0;
			if (OptionalParameter("LogDigiOut") != "")
			{
				if (mDigital)
					if (ReportError(DAQmxClearTask(mDigital)) < 0)
						bcierr << "Failed to clear existing task handling digital output" << endl;
				if (ReportError(DAQmxCreateTask("Digital_Output",&(TaskHandle)mDigital)) < 0)
					bcierr << "Unable to create task \"Digital_Output\" " << endl;
				if (ReportError(DAQmxCreateDIChan(mDigital,mActive[0].c_str(),"",DAQmx_Val_ChanForAllLines)) < 0)
					bcierr << "Unable to create channel operating on the following lines: \n" << mActive[0] << endl;
				if (ReportError(DAQmxClearTask(mDigital)) < 0)
					bcierr << "Failed to clear task \"Digital_Output\" " << endl;
			}
			if (OptionalParameter("LogAnaOut") != "")
			{
				float lMin = 0.0f;
				float lMax = 0.0f;
				if (mRanges.size() > 2)
				{
					lMin = mRanges[(int)OptionalParameter(string(mDevs[1]).append("OVRanges").c_str())*2];
					lMax = mRanges[((int)OptionalParameter(string(mDevs[1]).append("OVRanges").c_str())*2)+1];
				}
				else
				{
					lMin = mRanges[0];
					lMax = mRanges[1];
				}
				if (ReportError(DAQmxCreateLinScale("MilliVolts",1000.0,lMin*-2000.0,DAQmx_Val_Volts,"mV")) < 0)
					bcierr << "Failed to construct linear scale (MilliVolts)" << endl;
				if (mAnalog)
					if (ReportError(DAQmxClearTask(mAnalog)) < 0)
						bcierr << "Failed to clear existing task handling analog output" << endl;
				if (ReportError(DAQmxCreateTask("Analog_Output",&(TaskHandle)mAnalog)) < 0)
					bcierr << "Unable to create task \"Analog_Output\" " << endl;
				if (ReportError(DAQmxCreateAOVoltageChan(mAnalog,mActive[1].c_str(),"",lMin*1000,lMax*1000,DAQmx_Val_FromCustomScale,"MilliVolts")) < 0)
					bcierr << "Failed to create channel operating on the following lines: \n" << mActive[1] << endl;
				if (ReportError(DAQmxClearTask(mAnalog)) < 0)
					bcierr << "Failed to clear task \"Analog_Output\" " << endl;
			}
			for (int i = 0; i < (int)mLines.size(); i++)
				if (mLines[i])
					localCounter++;
			if (OptionalParameter("FilterExpressions") != "")
			{
				int localLoop = 0;
				if (OptionalParameter("FilterExpressions")->NumRows() == 0)
					bcierr << "Row count MUST be a positive integer" << endl;
				if (OptionalParameter("FilterExpressions")->NumColumns() > 1)
					bciout << "Only one column is needed for FilterExpressions. Only Expressions column will be read" << endl;
				for ( ; localLoop < OptionalParameter("FilterExpressions")->NumRows(); localLoop++)
				{
					Expression((string)(OptionalParameter("FilterExpressions")(localLoop,0))).Evaluate(&GenericSignal(Input));
					if ((string)OptionalParameter("FilterExpressions")(localLoop,0) == "0" && localLoop < localCounter)
						bciout << "Expression of value 0 detected on row " << localLoop << ". If intended, disregard warning" << endl;
				}
				if (localLoop < localCounter)
					bcierr << "Detected " << localLoop << "/" << localCounter << " rows in FilterExpressions Matrix. Please retry" << endl;
				if (localLoop > localCounter)
					bciout << "Too many rows detected. Filter will only use top " << localCounter << " row(s)" << endl;
			}
		}
		else // if no parameters were specified, tell the user that they are incorrectly using the Filter
			bcierr << "No parameters specified. Please read documentation on parameter specifications and try again" << endl;
	}
}
// Initialize everything that needs to be initialized //
void
NIDAQFilter::Initialize(const SignalProperties& Input, const SignalProperties& Output)
{
	if ((mUsed = (int)OptionalParameter("LogNIDAQout")) == true) // if the device is being used (and setting mDeviceEnable to true or false)
	{
		bool32	supported;
		// Reset Some Minor Things (We want to have a nice clean start whenever Initialize() is called) //
		mCounter[0] = mCounter[1] = 0;
		for (int i = 0; i < mFound[0]; i++)
			if (mLines[i])
				mCounter[0]++;
		if (mFound[0])
		{
			if (ReportError(DAQmxCreateTask("Digital_Output",&(TaskHandle)mDigital)) < 0)
				bcierr << "Unable to create task \"Digital_Output\" " << endl;
			if (ReportError(DAQmxCreateDIChan(mDigital,mActive[0].c_str(),"",DAQmx_Val_ChanForAllLines)) < 0)
				bcierr << "Failed to create channel operating on the following lines:\n" << mActive[0] << endl;
		}
		for (int i = mFound[0]; i < (int)mLines.size(); i++)
			if (mLines[i])
				mCounter[1]++;
		if (mFound[1])
		{
			float localMin = 0.0f;
			float localMax = 0.0f;
			if (mRanges.size() > 2)
			{
				localMin = mRanges[(int)OptionalParameter(string(mDevs[1]).append("OVRanges").c_str())*2];
				localMax = mRanges[((int)OptionalParameter(string(mDevs[1]).append("OVRanges").c_str())*2)+1];
			}
			else
			{
				localMin = mRanges[0];
				localMax = mRanges[1];
			}
			if (ReportError(DAQmxCreateTask("Analog_Output",&(TaskHandle)mAnalog)) < 0)
				bcierr << "Unable to create task \"Analog_Output\" " << endl;
			if (ReportError(DAQmxCreateAOVoltageChan(mAnalog,mActive[1].c_str(),"",localMin*1000,localMax*1000,DAQmx_Val_FromCustomScale,"MilliVolts")) < 0)
				bcierr << "Failed to create channel operating on the following lines:\n" << mActive[1] << endl;
			if (ReportError(DAQmxGetDevAOSampClkSupported(mDevs[1].c_str(),&supported)) >= 0 && supported)
				if (ReportError(DAQmxCfgSampClkTiming(mAnalog,"ao/SampleClockTimebase",mSampleRate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,10000)) < 0)
					bcierr << "Failed to set sampling rate on device " << mDevs[1] << endl;
		}
		for (int i = 0; i < mCounter[0]+mCounter[1]; i++)
			mExpressions.push_back(Expression((string)(Parameter("FilterExpressions")(i,0))));
	}
	mRan = true;
}
// Run the main loop //
void
NIDAQFilter::Process(const GenericSignal& Input, GenericSignal& Output)
{
	int localCounter	= 0;
	int localAnalog		= 0;
	uInt8 *lWrite = new uInt8[mCounter[0]*2];
	float64 *lFloatWrite = new float64[mCounter[1]*2];
	int32 lWritten;
	for (int i = 0; i < mFound[0]; i++)
		if (mLines[i])
		{
			lWrite[localCounter] = (uInt8)mExpressions[localCounter].Evaluate(&Input);
			bcievent << "NI" << mDevs[0] << "DOUTPUT" << i << " " << lWrite[localCounter++];
		}
	if (mCounter[0] && ReportError(DAQmxWriteDigitalLines(mDigital,mCounter[0],false,1.0,DAQmx_Val_GroupByScanNumber,lWrite,&lWritten,NULL)) < 0)
	{
		bcierr << "Failed to write to task \"Digital_Output\"" << endl;
		return;
	}
	for (int i = 0; i < mFound[1]; i++)
		if (mLines[i+mFound[0]])
		{
			lFloatWrite[localAnalog] = (uInt8)mExpressions[localCounter++].Evaluate(&Input);
			bcievent << "NI" << mDevs[0] << "AOUTPUT" << i << " " << lFloatWrite[localAnalog++];
		}
	if (mCounter[1] && ReportError(DAQmxWriteAnalogF64(mAnalog,mCounter[1],false,1.0,DAQmx_Val_GroupByScanNumber,lFloatWrite,&lWritten,NULL)) < 0)
	{
		bcierr << "Failed to write to task \"Analog_Output\"" << endl;
		return;
	}
    Output = Input;
}
// Begin running the main loop //
void
NIDAQFilter::StartRun()
{
  if (mAnalog && ReportError(DAQmxStartTask(mAnalog)) < 0)
    bcierr << "Failed to start task \"Analog_Output\" " << endl;
  if (mDigital && ReportError(DAQmxStartTask(mDigital)) < 0)
    bcierr << "Failed to start task \"Digital_Output\" " << endl;
}
// Stop running the main loop //
void
NIDAQFilter::StopRun()
{
  if (mRan && mAnalog != NULL)
  {
    if (ReportError(DAQmxStopTask(mAnalog)) < 0)
    bcierr << "Failed to stop task \"Analog_Output\" " << endl;
  }
  if (mRan && mDigital != NULL)
  {
    if (ReportError(DAQmxStopTask(mDigital)) < 0)
    bcierr << "Failed to stop task \"Digital_Output\" " << endl;
  }
}
// Halt the main loop //
void
NIDAQFilter::Halt() { StopRun(); }
// Report any NIDAQmx Errors that may occur //
int
NIDAQFilter::ReportError(int errCode) const
{
  if (DAQmxFailed(errCode)) // if the error code denotes that there is indeed an error, report it
  {
    char buffer[2048];
    DAQmxGetExtendedErrorInfo(buffer,2048);
    bcierr << "NIDAQ Error: " << buffer << endl;
    return errCode; // SOMETHING WENT WRONG HERE
  }
  return 1; // EVERYTHING IS OKAY
}
// Collect the device names (display them in operator log as well) //
vector<string>
NIDAQFilter::CollectDeviceNames()
{
  char	*localToken;
  char	localDevices[32];
  vector<string>	localDeviceList;
  if (ReportError(DAQmxGetSysDevNames(localDevices,32)) < 0)
  {
    bcierr << "Unable to detect any devices. Please make sure devices are properly connected to system and try again." << endl;
    return localDeviceList;
  }
  do
  {
    if (localDeviceList.size() == 0)
      localToken = strtok(localDevices,", ");
    else
      localToken = strtok(NULL,", ");
    if (localToken != '\0')
    {
      char	localInformation[32];
      localDeviceList.push_back(string(localToken));
      DAQmxGetDevProductType(string(localToken).c_str(),localInformation,32);
      bcidbg(0) << string(localToken) << " Product Type " << localInformation << endl;
    }
  } while (localToken != '\0');
  return localDeviceList;
}
// Get the number of digital lines on the specified device //
int
NIDAQFilter::GetNumDigitalLines(string device)
{
  int		lNumLines = 0;
  char	lLines[256];
  char*	lDeliminated;
  if (ReportError(DAQmxGetDevDILines(device.c_str(),lLines,256)) < 0) // if there is an error getting the available digital lines
  {
    bcierr << "Unable to detect digital lines. Make sure " << device << " is connected properly and try again." << endl;
    return -1;
  }
  do // do this... (tokenize recieved lines)
  {
    if (lNumLines == 0)
      lDeliminated = strtok(lLines,", ");
    else
      lDeliminated = strtok(NULL,", ");
    if (lDeliminated != '\0') // if we have not reached the end
    {
      lNumLines++;
      mLNames.push_back(lDeliminated);
    }
  } while (lDeliminated != '\0'); // ...until we reach the end
  return lNumLines;
}
// Get the number of analog output lines on the specified device //
int
NIDAQFilter::GetNumAnalogOutputLines(string device)
{
  int		lNumLines = 0;
  char	lLines[256];
  char*	lDeliminated;
  if (ReportError(DAQmxGetDevAOPhysicalChans(device.c_str(),lLines,256)) < 0)
  {
    bcierr << "Unable to detect analog output lines. Make sure " << device << " is connected properly and try again" << endl;
    return -1;
  }
  do
  {
    if (lNumLines == 0)
      lDeliminated = strtok(lLines,", ");
    else
      lDeliminated = strtok(NULL,", ");
    if (lDeliminated != '\0')
    {
      lNumLines++;
      mLNames.push_back(lDeliminated);
    }
  } while (lDeliminated != '\0');
  return lNumLines;
}
// Convert an integer into a string //
string
NIDAQFilter::IntToString(int n)
{
  // construct an ostringstream to hold the integer, then return .str()
  std::ostringstream result;
  result << n;
  return result.str();
}
// Convert a float into a string //
string
NIDAQFilter::FloatToString(float n)
{
  // construct an ostringstream to hold the float, then return .str()
  std::ostringstream result;
  result << n;
  return result.str();
}
// Acquire the analog output ranges for the device specified for analog output //
bool
NIDAQFilter::AcquireAOVRanges()
{
  float64	lAVRange[MAX_RANGES];	// the array of possible ranges
  if (mDevs[1] != "NULL")
  {
    if (ReportError(DAQmxGetDevAOVoltageRngs(mDevs[1].c_str(),lAVRange,MAX_RANGES)) < 0)	// try to get the voltage ranges for the device
    {
      bcierr << "Unable to obtain acceptable voltage ranges from specified device (" << mDevs[1] << ") " << endl;
      return false;
    }
    for (int i = 0; i < MAX_RANGES; i += 2)	// we want to check all of the possible pairs that we can for validity
      if (lAVRange[i+1] > 0) // if the 'high' value is more than zero, then it is valid
      {
        mRanges.push_back((float)lAVRange[i]);
        mRanges.push_back((float)lAVRange[i+1]);
      }
    return true;
  }
  else
  {
    bcierr << "Attempting to acquire voltage ranges without suitable device" << endl;
    return false;
  }
}
// determine if the string is inside of the given vector of strings //
bool
NIDAQFilter::find(string deviceName,vector<string> names)
{
  for (vector<string>::iterator itr = names.begin(); itr != names.end(); itr++)
    if ((*itr) == deviceName)	// if the devicename was found inside of names, return true!
      return true;
  return false;	// the end of the loop has been reached : the name wasn't found
}
