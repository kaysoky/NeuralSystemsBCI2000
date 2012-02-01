/////////////////////////////////////////////////////////////////////////////
// $Id: NIDAQLogger.cpp 2118 2008-09-04 15:36:18Z mellinger $              //
// Author: justin.renga@gmail.com                                          //
// Description: A logger for National Instruments Data Acquisition Boards  //
//                                                                         //
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
/////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop
#include "NIDAQLogger.h"
#include "BCIEvent.h"
#include "PrecisionTime.h"
#include <sstream>
using namespace std;
Extension( NIDAQLogger );
// The constructor for the NIDAQLogger //
NIDAQLogger::NIDAQLogger()
: mUsed( false ),
  mSampleRate( 1.0 ),
  mAnalog( NULL ),
  mDigital( NULL ),
  mDigiBuff( NULL ),
  mAnaBuff( NULL )
{
	mFound[0] = mFound[1] = 0;
	mCounter[0] = mCounter[1] = 0;
	mDevs[0] = mDevs[1] = "NULL";
	mActive[0] = mActive[1] = "";
}
// the destructor for the NIDAQLogger //
NIDAQLogger::~NIDAQLogger()
{
	if (mAnalog)
		if (ReportError(DAQmxClearTask(mAnalog)) < 0)
			bcierr << "Unable to clear analog task" << endl;
	if (mDigital)
		if (ReportError(DAQmxClearTask(mDigital)) < 0)
			bcierr << "Unable to clear digital task" << endl;
	mLines.clear();
	mRanges.clear();
	mLNames.clear();
	if (mDigiBuff)
		delete mDigiBuff;
	if (mAnaBuff)
		delete mAnaBuff;
}
// Report any NIDAQmx errors that may occur //
int
NIDAQLogger::ReportError(int error) const
{
	if (DAQmxFailed(error)) // if the error code denotes that there is indeed an error, report it
	{
		char buffer[2048];
		DAQmxGetExtendedErrorInfo(buffer,2048);
		bcierr << "NIDAQ Error: " << buffer << endl;
		return error; // SOMETHING WENT WRONG HERE
	}
	return 1; // EVERYTHING IS OKAY
}
// Returns the acceptable voltage ranges for analog input //
bool
NIDAQLogger::AcquireAIVRanges()
{
	float64	lAVRange[MAX_RANGES];
	if (mDevs[1] != "NULL")
	{
		if (ReportError(DAQmxGetDevAIVoltageRngs(mDevs[1].c_str(),lAVRange,MAX_RANGES)) < 0)
		{
			bcierr << "Unable to obtain acceptable voltage ranges from specified device (" << mDevs[1] << ") " << endl;
			return false;
		}
		for (int i = 0; i < MAX_RANGES; i += 2)
		{
			if (lAVRange[i+1] > 0) // if the 'high' value is more than zero, then it is valid
			{
				mRanges.push_back((float)lAVRange[i]);
				mRanges.push_back((float)lAVRange[i+1]);
			}
		}
		return true;
	}
	else
	{
		bcierr << "Attempting to acquire voltage ranges without suitable device" << endl;
		return false;
	}
}
// Get the number of digial lines supported by the given device //
int
NIDAQLogger::GetNumDigitalLines(std::string dev_name)
{
	int		lNumLines = 0;
	char	lLines[256];
	char*	lDeliminated;
	if (ReportError(DAQmxGetDevDILines(dev_name.c_str(),lLines,256)) < 0) // if there is an error getting the available digital lines
	{
		bcierr << dev_name << " is not a valid device name. Please make sure device is typed in correctly and try again" << endl; // report it
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
			mLNames.push_back(lDeliminated);
			lNumLines++;
		}
	} while (lDeliminated != '\0'); // ...until we reach the end
	return lNumLines;
}
// Returns the number of Analog Input Lines supported by the device. //
int
NIDAQLogger::GetNumAnalogInputLines(string dev_name)
{
	int		lNumLines = 0;
	char	lLines[256];
	char*	lDeliminated;
	if (ReportError(DAQmxGetDevAIPhysicalChans(dev_name.c_str(),lLines,256)) < 0) // if we have encountered an error when attempting to get the names of the lines, say something about it
	{
		bcierr << dev_name << " is not a valid device name. Please make sure device is typed in correctly and try again" << endl;
		return -1;
	}
	do // now that we have the list of lines, we need to determine the number of items in the list by seperating the spaces and commas
	{
		if (lNumLines == 0)
			lDeliminated = strtok(lLines,", ");
		else
			lDeliminated = strtok(NULL,", ");
		if (lDeliminated != '\0') // if we have not yet reached the end of the string, we will add one to our count
		{
			mLNames.push_back(lDeliminated);
			lNumLines++;
		}
	} while (lDeliminated != '\0'); // we will continue this loop until we have reached the end of the string
	return lNumLines; // return what we have found
}

// Returns a string version of integer argument //
string
NIDAQLogger::IntToString(int n)
{
	std::ostringstream result;
	result << n;
	return result.str();
}
// Returns a string version of float argument //
string
NIDAQLogger::FloatToString(float n)
{
	std::ostringstream lResult;
	lResult << n;
	return lResult.str();
}
// determines if the device is in the vector //
bool
NIDAQLogger::find(string deviceName,vector<string> names)
{
	for (vector<string>::iterator itr = names.begin(); itr != names.end(); itr++)
		if ((*itr) == deviceName)
			return true;
	return false;
}
// returns all of the devices connected to the computer //
vector<string>
NIDAQLogger::CollectDeviceNames()
{
	char	*lToken;
	char	lDevices[32];
	vector<string>	lDeviceList;
	if (ReportError(DAQmxGetSysDevNames(lDevices,32)) < 0)
	{
		bcierr << "Unable to detect any devices. Please make sure devices are properly conencted" << endl;
		return lDeviceList;
	}
	do
	{
		if (lDeviceList.size() == 0)
			lToken = strtok(lDevices,", ");
		else
			lToken = strtok(NULL,", ");
		if (lToken != '\0')
		{
			char	lInformation[32];
			lDeviceList.push_back(string(lToken));
			DAQmxGetDevProductType(string(lToken).c_str(),lInformation,32);
			bcidbg(0) << string(lToken) << " Product Type " << lInformation << endl;
		}
	} while (lToken != '\0');
	return lDeviceList;
}
// Callback for digital task //
int32
NIDAQLogger::DigitalCallback(TaskHandle handle, int32 everyNSamplesEventType, uInt32 nSamples, void *callbackData)
{
	NIDAQLogger* pLogger = static_cast<NIDAQLogger*>(callbackData);
	int lCounter = 0;
	pLogger->GetDigitalData();
	for (int lLoop = 0; lLoop < pLogger->mFound[0]; lLoop++)
		if (pLogger->mLines[lLoop])
			bcievent << "NI" << pLogger->mDevs[0] << "DINPUT" << lLoop << " " << pLogger->mAnaBuff[lCounter++];
	return DAQmxSuccess;
}
// Grab data for digital lines //
void
NIDAQLogger::GetDigitalData()
{
	int32 lSamplesPerChannelRead;
	int32 lNumBytesPerSample;
	if (ReportError(DAQmxReadDigitalLines(mDigital,mCounter[0],DAQmx_Val_WaitInfinitely,DAQmx_Val_GroupByScanNumber,mDigiBuff,mCounter[0],&lSamplesPerChannelRead,&lNumBytesPerSample,NULL)) < 0)
		bcierr << "Failed to read digital lines" << endl;
}
// Callback for analog task //
int32
NIDAQLogger::AnalogCallback(TaskHandle handle, int32 everyNSamplesEventType, uInt32 nSamples, void *callbackData)
{
	NIDAQLogger* pLogger = static_cast<NIDAQLogger*>(callbackData);
	int lCounter = 0;
	pLogger->GetAnalogData();
	for (int lLoop = 0; lLoop < pLogger->mFound[1]; lLoop++)
		if (pLogger->mLines[lLoop+pLogger->mFound[0]])
  			bcievent << "NI" << pLogger->mDevs[1] << "AINPUT" << lLoop << " " << pLogger->mAnaBuff[lCounter++];
	return DAQmxSuccess;
}
// Grab data for analog lines //
void
NIDAQLogger::GetAnalogData()
{
	int32 lSamplesPerChanRead;
	if (ReportError(DAQmxReadAnalogF64(mAnalog,mCounter[1],DAQmx_Val_WaitInfinitely,DAQmx_Val_GroupByScanNumber,mAnaBuff,mCounter[1],&lSamplesPerChanRead,NULL)) < 0)
		bcierr << "Failed to read analog lines" << endl;
}
// Parses Commands and creates states/parameters //
void
NIDAQLogger::Publish()
{
	if (OptionalParameter("LogNIDAQin") == 1)
	{
		// Collect Device Names //
		vector<string>		lDevices;
		char				lPValue[32];	// the full parameter value
		unsigned long int	lSerial = 0;	// the serial number of the device (to test mx compatibility)
		string				lDevice;		// the device name
		string				lParam;			// the ports (and other paramters)
		if ((lDevices = CollectDeviceNames()).empty()) return; // error message already dealt with
		if (OptionalParameter("LogDigiIn") != "")
		{
			strncpy(lPValue,((string)OptionalParameter("LogDigiIn")).c_str(),((string)OptionalParameter("LogDigiIn")).size()+1);
			mDevs[0]	=	strtok(lPValue,"-");
			lParam	=	strtok(NULL,"-");
			DAQmxGetDevSerialNum(lDevice.c_str(),&lSerial);
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
				bcierr << "No digital lines were detected on " << lDevice << endl;
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
				mLines.push_back(charVal);
				if (charVal)
				{
					if (!mActive[0].empty())
						mActive[0].append(", ");
					mActive[0].append(mLNames[i]);
					mCounter[0]++;
				}
			}
		}
		if ((OptionalParameter("LogAnaIn") != ""))
		{
			strncpy(lPValue,((string)OptionalParameter("LogAnaIn")).c_str(),((string)OptionalParameter("LogAnaIn")).size()+1);
			mDevs[1] = strtok(lPValue,"-");
			lParam = strtok(NULL,"-");
			bcidbg(0) << mDevs[1] << endl;
			DAQmxGetDevSerialNum(mDevs[1].c_str(),&lSerial);
			if (lSerial == 0)
			{
				bcierr << mDevs[1] << " is either not connected properly or does not support DAQmx 8.x . Check connections and try again" << endl;
				return;
			}
			if (!find(mDevs[1],lDevices))
			{
				bcierr << mDevs[1] << " is not connected to the computer. Please check connections and try again" << endl;
				return;
			}
			if ((mFound[1] = GetNumAnalogInputLines(mDevs[1])) < 0)
			{
				bcierr << "No analog input lines were detected on " << mDevs[1] << endl;
				return;
			}
			if (lParam.size() != mFound[1])
			{
				bcierr	<< "Unequal lengths detected. Parameter length (" << lParam.size() << ") does not match number of physical channels (" <<
						mFound[1] << ")" << endl;
				return;
			}
			// Go through each item in the port list -> are all of the characters valid? If one isn't, throw an error and quit //
			for (int i = 0; i < (int)lParam.size(); i++)
			{
				int lCharVal = lParam.at(i)-48;
				if (lCharVal < 0 || lCharVal > 1)
				{
					bcierr << "Invalid character detected at port " << i << ". Valid characters and functions are listed below:\n" <<
						"0: Port is not being used.\n1: Port is being used for input purposes.\nPlease correct parameter list and try again" << endl;
					return;
				}
				mLines.push_back(lCharVal);
				if (lCharVal)
				{
					if (!mActive[1].empty())
						mActive[1].append(", ");
					mActive[1].append(mLNames[i]);
				}
			}
			if (!AcquireAIVRanges()) return;		// error message already taken care of inside function
			if (!mRanges.empty() && mRanges.size() > 2)
			{
				string lParameter = "Source:NILogger int ";
				lParameter.append(mDevs[1]);
				lParameter.append("IVRanges= 0 0 0 ");
				lParameter.append(IntToString((mRanges.size()/2)-1));
				lParameter.append(" // Support Input Voltage Ranges ");
				for (int i = 0; i < (int)mRanges.size(); i += 2)
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
		if (mDevs[1] != "NULL")
		{
			string lParameter = "Source:NILogger float ";
			lParameter.append(mDevs[1].c_str());
			lParameter.append("InSamplingRate= 256.0 256.0 1.0 16384.0 // input sampling rate of ");
			lParameter.append(mDevs[1].c_str());
			lParameter.append(" (ranges between 1Hz and 16384Hz)");
			BEGIN_PARAMETER_DEFINITIONS
				lParameter.c_str(),
			END_PARAMETER_DEFINITIONS
		}
		for (int i = 0; i < (int)mLines.size(); i++)
		{
			if (mLines[i])
			{
				string lStates = "NI";
				lStates.append((i < mFound[0] ? mDevs[0].c_str() : mDevs[1].c_str()));
				lStates.append((i < mFound[0] ? "DINPUT" : "AINPUT"));
				lStates.append(IntToString((i < mFound[0] ? i : i-mFound[0])));
				lStates.append((i < mFound[0] ? " 1 0 0 0" : " 16 0 0 0"));
				BEGIN_EVENT_DEFINITIONS
					lStates.c_str(),
				END_EVENT_DEFINITIONS
			}
		}
	}
}
// PREFLIGHT //
void
NIDAQLogger::Preflight() const
{
	if (OptionalParameter("LogNIDAQin") > 0) // is the device being used?
	{
		if (!mLines.empty()) // is mLines empty()?
		{
			for (int i = 0; i < 2; i++)
				if (mDevs[i] != "NULL")
					if ((float)Parameter(string(mDevs[i]).append("InSamplingRate").c_str()) > 256.0)
						bciout << "Device sample rate is above 256. Some lag may occur" << endl;
			if (OptionalParameter("LogDigiIn") != "")
			{
				if (mDigital)
					if (ReportError(DAQmxClearTask(mDigital)) < 0)
						bcierr << "Failed to clear existing task handling digital input" << endl;
				if (ReportError(DAQmxCreateTask("Digital_Input",&(TaskHandle)mDigital)) < 0)
					bcierr << "Unable to create task \"Digital_Input\" " << endl;
				if (ReportError(DAQmxCreateDIChan(mDigital,mActive[0].c_str(),"",DAQmx_Val_ChanForAllLines)) < 0)
					bcierr << "Unable to create channel operating on the following lines: \n" << mActive[0] << endl;
				if (ReportError(DAQmxClearTask(mDigital)) < 0)
					bcierr << "Failed to clear task \"Digital_Input\" " << endl;
			}
			if (OptionalParameter("LogAnaIn") != "")
			{
				float lMin = 0.0;
				if (mRanges.size() > 2)
					lMin = mRanges[(int)OptionalParameter(string(mDevs[1]).append("IVRanges").c_str())*2];
				else
					lMin = mRanges[0];
				if (ReportError(DAQmxCreateLinScale("MilliVolts",1000.0,lMin*-2000.0,DAQmx_Val_Volts,"mV")) < 0)
					bcierr << "Failed to construct linear scale (MilliVolts)" << endl;
				if (mAnalog)
					if (ReportError(DAQmxClearTask(mAnalog)) < 0)
						bcierr << "Failed to clear existing task handling analog input" << endl;
				if (ReportError(DAQmxCreateTask("Analog_Input",&(TaskHandle)mAnalog)) < 0)
					bcierr << "Unable to create task \"Analog_Input\" " << endl;
				if (ReportError(DAQmxCreateAIVoltageChan(mAnalog,mActive[1].c_str(),"",DAQmx_Val_RSE,-lMin*1000,-lMin*3000,DAQmx_Val_FromCustomScale,"MilliVolts")) < 0)
					bcierr << "Failed to create channel operating on the following lines: \n" << mActive[1] << endl;
				if (ReportError(DAQmxClearTask(mAnalog)) < 0)
					bcierr << "Failed to clear task \"Analog_Input\" " << endl;
			}
		}
		else // if no parameters were specified, tell the user that there is a problem
		{
			bcidbg(0) << mLines.size() << endl;
			bcierr << "No parameters specified. Please read documentation on parameter specifications and try again" << endl;
		}
	}
}
// INITIALIZE //
void
NIDAQLogger::Initialize()
{
	if ((mUsed = (int)OptionalParameter("LogNIDAQin")) == true) // if the device is being used (and setting mDeviceEnable to true or false)
	{
		// Reset Some Minor Things (We want to have a nice clean start whenever Initialize() is called) //
		mCounter[0] = mCounter[1] = 0;
		if (mDigiBuff)
			delete mDigiBuff;
		if (mAnaBuff)
			delete mAnaBuff;
		mSampleRate = (float)Parameter(string(mDevs[1]).append("InSamplingRate"));
		for (int lLoop = 0; lLoop < mFound[0]; lLoop++)
			if (mLines[lLoop])
				mCounter[0]++;
		if (mFound[0])
		{
			if (ReportError(DAQmxCreateTask("Digital_Input",&(TaskHandle)mDigital)) < 0)
				bcierr << "Unable to create task \"Digital_Input\" " << endl;
			if (ReportError(DAQmxCreateDIChan(mDigital,mActive[0].c_str(),"",DAQmx_Val_ChanForAllLines)) < 0)
				bcierr << "Failed to create channel operating on the following lines:\n" << mActive[0] << endl;
			if (ReportError(DAQmxRegisterEveryNSamplesEvent(mDigital,DAQmx_Val_Acquired_Into_Buffer,mCounter[0],0,(DAQmxEveryNSamplesEventCallbackPtr) &DigitalCallback,this)) < 0)
					bcierr << "Failed to associate \"Digital_Input\" task with callback function DigitalCallback() " << endl;
		}
		for (int lLoop = mFound[0]; lLoop < (int)mLines.size(); lLoop++)
			if (mLines[lLoop])
				mCounter[1]++;
		if (mFound[1])
		{
			float lMin = 0.0;
			if (mRanges.size() > 2)
				lMin = mRanges[(int)OptionalParameter(string(mDevs[1]).append("IVRanges").c_str())*2];
			else
				lMin = mRanges[0];
			if (ReportError(DAQmxCreateTask("Analog_Input",&(TaskHandle)mAnalog)) < 0)
				bcierr << "Unable to create task \"Analog_Input\" " << endl;
			if (ReportError(DAQmxCreateAIVoltageChan(mAnalog,mActive[1].c_str(),"",DAQmx_Val_RSE,-lMin*1000,-lMin*3000,DAQmx_Val_FromCustomScale,"MilliVolts")) < 0)
				bcierr << "Failed to create channel operating on the following lines:\n" << mActive[1] << endl;
			if (ReportError(DAQmxCfgSampClkTiming(mAnalog,"ai/SampleClockTimebase",mSampleRate,DAQmx_Val_Rising,DAQmx_Val_ContSamps,16384)) < 0)
				bcierr << "Failed to set sampling rate on device " << mDevs[1] << endl;
			if (ReportError(DAQmxRegisterEveryNSamplesEvent(mAnalog,DAQmx_Val_Acquired_Into_Buffer,mCounter[1],0,(DAQmxEveryNSamplesEventCallbackPtr) &AnalogCallback,this)) < 0)
					bcierr << "Failed to associate \"Analog_Input\" task with callback function AnalogCallback() " << endl;
		}
	}
}
// Start Tasks //
void
NIDAQLogger::StartRun()
{
	if (mAnalog)
		if (ReportError(DAQmxStartTask(mAnalog)) < 0)
			bcierr << "Failed to start \"Analog_Input\" task" << endl;
	if (mDigital)
		if (ReportError(DAQmxStartTask(mDigital)) < 0)
			bcierr << "Failed to start \"Digital_Input\" task" << endl;
	if (mCounter[0])
		mDigiBuff = new uInt8[mCounter[0]*2];
	if (mCounter[1])
		mAnaBuff = new float64[mCounter[1]*2];
}
// Processes the main event loop //
void
NIDAQLogger::Process()
{
	// NOTHING WILL BE DONE HERE [IT IS ALL DONE IN THE CALLBACK] //
}
// If the task is stopped... //
void
NIDAQLogger::StopRun()
{
	if (mAnalog)
		if (ReportError(DAQmxStopTask(mAnalog)) < 0)
			bcierr << "Failed to stop \"Analog_Input\" task" << endl;
	if (mDigital)
		if (ReportError(DAQmxStopTask(mDigital)) < 0)
			bcierr << "Failed to stop \"Digital_Input\" task" << endl;
	if (mDigiBuff)
		delete mDigiBuff;
	if (mAnaBuff)
		delete mAnaBuff;
	mDigiBuff = NULL;
	mAnaBuff = NULL;
}
// If the task is halted... //
void
NIDAQLogger::Halt() { StopRun(); }
