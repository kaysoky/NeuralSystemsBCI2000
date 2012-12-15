////////////////////////////////////////////////////////////////////$
// Authors: Paul Ignatenko <paul dot ignatenko at gmail dot com
// Description: Device controls for the actiCHamp source module
//
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
// You should have received a copy of the GNU General Public License along with // this program.  If not, see <http://www.gnu.org/licenses/>.  //
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "actiCHampDevice.h"
#include "BCIError.h"
#include <windows.h>
#include <sstream>
#include <iostream>


#define _ACTICHAMP_ERROR_STREAM bcierr
#define _ACTICHAMP_OUTPUT_STREAM bciout


using namespace std;


actiCHampDevice::actiCHampDevice()
{    
}

actiCHampDevice::~actiCHampDevice()
{
    this->close();
}



/**
 * Open a device by device number
 *
 * Opens a device and tries to intitialize the device, library, and FPGA has
 * library_settings.init_tries to init the FPGA. The FPGA should init on first
 * try unless the file ActiChamp.bit is missing.
 *
 * @param devNum device number to initialize
 *
 * @return true if device initialized properly false otherwise
 */
bool actiCHampDevice::open(int devNum)
{
    int e;
    //Get the number of devices, with this we can test if we can see hardware.
    if(champGetCount() == 0)
    {
        _ACTICHAMP_ERROR_STREAM << "Hardware is not available" << endl;
        return false;
    }

    device = champOpen(devNum);

    if(device == 0)
    {
        _ACTICHAMP_ERROR_STREAM << "Failed to get device handle." << endl;
        return false;
    }

    e = champGetVersion(device, &device_settings.version);
    if (e != CHAMP_ERR_OK) {
        _ACTICHAMP_ERROR_STREAM << "Could not get device version. actiCHamp Error: " << e << endl;
        return false;
    }
    
    if (device_settings.version.Fpga == 0) {
        if (library_settings.init_tries < 1)
        {
            _ACTICHAMP_ERROR_STREAM << "FPGA failed to initalize... exiting" << endl;
            return false;
        }
        else
        {
            _ACTICHAMP_ERROR_STREAM << "FPGA not initialized on this try, make sure that the firmware file is in the same directory as the DLL." << endl << "Retrying to open the device...."<< endl;
            close();
            --library_settings.init_tries;
            return open(devNum);
        }
    }

    library_settings.init_tries = 3;

    e = champGetModules(device, &device_settings.modules);
    if (e != CHAMP_ERR_OK) {
        _ACTICHAMP_ERROR_STREAM << "Could not get device modules. actiCHamp Error: " << e << endl;
        return false;
    }


    e = champGetProperty(device, &device_settings.properties);
    if (e != CHAMP_ERR_OK) {
        _ACTICHAMP_ERROR_STREAM << "Could not get device properties. actiCHamp Error: " << e << endl;
        return false;
    }

    t_champSettingsEx temp_settings;
    e = champGetSettingsEx(device, &temp_settings);
    if( e != CHAMP_ERR_OK)
    {
        _ACTICHAMP_ERROR_STREAM << "Could not get device default settings. actiCHamp Error: " << e << endl;
        return false;
    }

    device_settings.mode = temp_settings.Mode;
    device_settings.rate = temp_settings.Rate;
    device_settings.averaging = temp_settings.AdcFilter;
    device_settings.decimation = temp_settings.Decimation;

    library_settings.acquisition_lock = CreateMutex(NULL, false, NULL);

    if(library_settings.acquisition_lock == NULL)
    {
        _ACTICHAMP_ERROR_STREAM << "CreateMutex error..." << GetLastError();
        return false;
    }


    return true;
}



/**
 * Closes the device
 *
 * Closes the device
 *
 * @return None
 */
void actiCHampDevice::close()
{
    //Acquire mutex first...
    if(library_settings.acquiring_data == true)
    {
        _ACTICHAMP_OUTPUT_STREAM <<"Device still in data acquisition mode, stop the device before closing.\n";
    } 
    else if(device != NULL || device != 0)
    {
        champClose(device);
        device = 0;
        CloseHandle(library_settings.acquisition_lock);

    }


}


bool actiCHampDevice::init() 
{
    //Enable our modules
    device_settings.modules.Enabled = device_settings.modules.Present;
	t_champModules temp_modules = device_settings.modules;
    int e = champSetModules(device, &temp_modules);

    if (e != CHAMP_ERR_OK) 
    {
        _ACTICHAMP_ERROR_STREAM << "Error could not set modules correctly. Error: " << e <<endl;
        return false;
    }

    //What dataformat do we use?
    if((device_settings.modules.Enabled &  (0x20 | 0x10 | 0x08 | 0x04 | 0x02 | 0x01)) == (0x20 | 0x10 | 0x08 | 0x04 | 0x02 | 0x01))
        library_settings.number_of_channels = 168;
    else if((device_settings.modules.Enabled &  (0x10 | 0x08 | 0x04 | 0x02 | 0x01)) == (0x10 | 0x08 | 0x04 | 0x02 | 0x01) )
        library_settings.number_of_channels = 136;
    else if((device_settings.modules.Enabled &  (0x08 | 0x04 | 0x02 | 0x01)) ==  (0x08 | 0x04 | 0x02 | 0x01))
        library_settings.number_of_channels = 104;
	else if((temp_modules.Enabled &  (0x04 | 0x02 | 0x01)) == (0x04 | 0x02 | 0x01) )
        library_settings.number_of_channels = 72;
    else if((device_settings.modules.Enabled &  (0x02 | 0x01)) == (0x02 | 0x01) )
        library_settings.number_of_channels = 40;
    else if((device_settings.modules.Enabled &  (0x01)) == (0x01) )
        library_settings.number_of_channels = 8;
    else
        _ACTICHAMP_ERROR_STREAM << "Unknown number of modules active" << endl;
        


    //Set our desired settings.
    t_champSettingsEx champ_init_settings;
    champ_init_settings.Mode       = device_settings.mode;
    champ_init_settings.Rate       = device_settings.rate;
    champ_init_settings.AdcFilter  = device_settings.averaging;
    champ_init_settings.Decimation = device_settings.decimation;

    e = champSetSettingsEx(device, &champ_init_settings); 
    if (e != CHAMP_ERR_OK) 
    {
        _ACTICHAMP_ERROR_STREAM << "Error could not set settings correctly. Error: " << e <<endl;
        return false;
    }

    e = champGetProperty(device, &device_settings.properties);
    if (e != CHAMP_ERR_OK) {
        _ACTICHAMP_ERROR_STREAM << "Could not get device properties. actiCHamp Error: " << e << endl;
        return false;
    }

    return true;
}


/**
 * Initialize the device for data aquisition
 *
 * Tries to set the parameters for data aquisition according to the user
 * settings.
 *
 * @return True if the preperation was successful, false otherwise.
 */

bool actiCHampDevice::start()
{

    int e;

    // Start our amplifier
    e = champStart(device);
    if (e != CHAMP_ERR_OK) 
    {
        _ACTICHAMP_ERROR_STREAM << "Error could not start the device correctly. Error: " << e <<endl;
        return false;
    }


    library_settings.acquiring_data = true;
    return true;
}



/**
 * Takes device out of data acquisition mode
 *
 * Stops data acquisition on the device, the device will no longer continually
 * read data and fill it's buffer
 *
 * @return true if successful false otherwise
 */
bool actiCHampDevice::stop()
{
    if (!library_settings.acquiring_data)
    {
        _ACTICHAMP_ERROR_STREAM << "Error device not in data acquisition mode, it is already in stopped state.";
        return false;
    }
    else
    {
        int lock_status = WaitForSingleObject(library_settings.acquisition_lock, 6000);
        int e;
        switch(lock_status)
        {
            case WAIT_OBJECT_0:
                e = champStop(device);
                if (e != CHAMP_ERR_OK) 
                {
                    _ACTICHAMP_ERROR_STREAM << "Error could not stop the device correctly. Error: " << e <<endl;
                    return false;
                }

                library_settings.acquiring_data = false;
                return true;
                break;

            case WAIT_TIMEOUT:
            case WAIT_FAILED:
                    _ACTICHAMP_ERROR_STREAM << "Error could not stop the device correctly. Could not acquire aquisition handle" << endl;
                    return false;
                break;

        }
        return false;
    }
}



/**
 * Gets a certain ammount of data from the device
 *
 * Blocks until it collects a certain ammount of samples (<size>) from the device
 *  
 * @param output array of <size> where the collected data will go.
 * @param size the ammount of samples to collect
 *
 * @return return output filled with samples collected from the amp
 */
void actiCHampDevice::get_data(GenericSignal & output, unsigned int size_in_samples)
{  
    int size_in_bytes = (device_settings.properties.CountEeg + device_settings.properties.CountAux + 2) * sizeof(int) * size_in_samples;
    if (library_settings.number_of_channels == 8)
    {
        dataaux = new t_champDataModelAux[size_in_samples];

        get_data_helper(dataaux, size_in_bytes);

        for( unsigned int ch = 0; ch < (unsigned int)output.Channels(); ch++ )
        {
             for( int el = 0; el < output.Elements(); el++ )
             {
                 output( ch, el ) = get_channel(dataaux[el], ch);
                 output(ch,el) = output(ch,el) * device_settings.properties.ResolutionAux;
             }
        }

        delete[] dataaux;
    }
    else if (library_settings.number_of_channels == 40)
    {
        data32 = new t_champDataModel32[size_in_samples];


        get_data_helper(data32, size_in_bytes);

        for( unsigned int ch = 0; ch < (unsigned int)output.Channels(); ch++ )
        {
             for( int el = 0; el < output.Elements(); el++ )
             {

                if(ch >= library_settings.number_of_channels -  8)
                 {
                     output( ch, el ) = get_channel(data32[el], ch);
                     output(ch,el) = output(ch,el) * device_settings.properties.ResolutionAux;
                 }
                 else
                 {
                     output( ch, el ) = get_channel(data32[el], ch) - get_channel(data32[el], library_settings.reference_channel); 
                     output(ch,el) = output(ch,el) * device_settings.properties.ResolutionEeg;
                 }

             }
        }
        delete[] data32;
    }
    else if (library_settings.number_of_channels == 72)
    {
         data64 = new t_champDataModel64[size_in_samples];

         get_data_helper(data64, size_in_bytes);

        for( unsigned int ch = 0; ch < (unsigned int)output.Channels(); ch++ )
        {
             for( int el = 0; el < output.Elements(); el++ )
             {
                if(ch >= library_settings.number_of_channels -  8)
                 {
                     output( ch, el ) = get_channel(data64[el], ch);
                     output(ch,el) = output(ch,el) * device_settings.properties.ResolutionAux;
                 }
                 else
                 {
                     output( ch, el ) = get_channel(data64[el], ch) - get_channel(data64[el], library_settings.reference_channel); 
                     output(ch,el) = output(ch,el) * device_settings.properties.ResolutionEeg;
                 }
             }
        }
        delete[] data64;
    }
    else if (library_settings.number_of_channels == 104)
    {
        data96 = new t_champDataModel96[size_in_samples];
         
        get_data_helper(data96, size_in_bytes);

        for( unsigned int ch = 0; ch < (unsigned int)output.Channels(); ch++ )
        {
             for( int el = 0; el < output.Elements(); el++ )
             {
                if(ch >= library_settings.number_of_channels -  8)
                 {
                 output( ch, el ) = get_channel(data96[el], ch);
                     output(ch,el) = output(ch,el) * device_settings.properties.ResolutionAux;
                 }
                 else
                 {
                     output( ch, el ) = get_channel(data96[el], ch) - get_channel(data96[el], library_settings.reference_channel); 
                     output(ch,el) = output(ch,el) * device_settings.properties.ResolutionEeg;
                 }
             }
        }
        delete[] data96;

    }
    else if (library_settings.number_of_channels == 136)
    {
        data128 = new t_champDataModel128[size_in_samples];

        get_data_helper(data128, size_in_bytes);

        for( unsigned int ch = 0; ch < (unsigned int)output.Channels(); ch++ )
        {
             for( int el = 0; el < output.Elements(); el++ )
             {
                if(ch >= library_settings.number_of_channels -  8)
                 {
                     output( ch, el ) = get_channel(data128[el], ch); 
                     output(ch,el) = output(ch,el) * device_settings.properties.ResolutionAux;
                 }
                 else
                 {
                     output( ch, el ) = get_channel(data128[el], ch) - get_channel(data128[el], library_settings.reference_channel); 
                     output(ch,el) = output(ch,el) * device_settings.properties.ResolutionEeg;
                 }
             }
        }
        delete[] data128;
    }
    else if (library_settings.number_of_channels == 168)
    {
        data160 = new t_champDataModel160[size_in_samples];
        
        get_data_helper(data160, size_in_bytes);

        for( unsigned int ch = 0; ch < (unsigned int)output.Channels(); ch++ )
        {
             for( int el = 0; el < output.Elements(); el++ )
             {
                if(ch >= library_settings.number_of_channels -  8)
                 {
                     output( ch, el ) = get_channel(data160[el], ch);
                     output(ch,el) = output(ch,el) * device_settings.properties.ResolutionAux;
                 }
                 else
                 {
                     output( ch, el ) = get_channel(data160[el], ch) - get_channel(data160[el], library_settings.reference_channel); 
                     output(ch,el) = output(ch,el) * device_settings.properties.ResolutionEeg;
                 }
             }
        }
        delete[] data160;
    }
    else{
        _ACTICHAMP_ERROR_STREAM << "Uknown number of modules present" <<endl ;
    }


}

void actiCHampDevice::get_data_helper(void* buffer , unsigned int size)
{
    int e;
    int wait_result = WaitForSingleObject(library_settings.acquisition_lock, 2000);
    switch(wait_result)
    {
        case WAIT_OBJECT_0:
            e = champGetDataBlocking(device, buffer, size);
            if (e < 0)
            {
                _ACTICHAMP_ERROR_STREAM << "Error acquiring data" <<endl;
            }
            if (!ReleaseMutex(library_settings.acquisition_lock))
            {
                _ACTICHAMP_ERROR_STREAM << "Could not release acquisition lock!" <<endl;
            }
            break;
        case WAIT_TIMEOUT:
        case WAIT_FAILED:
            _ACTICHAMP_OUTPUT_STREAM << "Could not acquire mutex for acquisition, device must be terminating. This message is normal." <<endl;
            break;
    }

}

t_champDataStatus actiCHampDevice::get_data_status() const
{
    t_champDataStatus champ_data_info;
    int e = champGetDataStatus(device, &champ_data_info);
    if (e != CHAMP_ERR_OK)
    {
        _ACTICHAMP_ERROR_STREAM << "Could not get device data status. actiCHamp data: " << e << endl;
    }
    return champ_data_info;
}

t_champErrorStatus actiCHampDevice::get_error_status() const
{
    t_champErrorStatus champ_error_info;
    int e = champGetErrorStatus(device, &champ_error_info);
    if (e != CHAMP_ERR_OK)
    {
        _ACTICHAMP_ERROR_STREAM << "Could not get device error status. actiCHamp Error: " << e << endl;
    }
    return champ_error_info;
}

bool actiCHampDevice::Settings(string& settings) const
{
    t_champSettingsEx temp_settings;
    int e = champGetSettingsEx(device, &temp_settings);
    if( e != CHAMP_ERR_OK)
    {
        _ACTICHAMP_ERROR_STREAM << "Could not get device settings. actiCHamp Error: " << e << endl;
        return false;
    }
    else
    {
        stringstream ss;
        ss << "Device Settings: "<<  "\n"
            << " Rate = " << temp_settings.Rate << "\n"
            << " Mode = " << temp_settings.Mode << "\n"
            << " Averaging = " << temp_settings.AdcFilter << "\n"
            << " Decimation = " << temp_settings.Decimation << "\n"
            << endl;
        settings = ss.str();
    }
    return true;

}

/**
 * Get the data status of the device into ostream output
 *
 * Gets the device data status, and outputs it in to a ostream in a text
 * readable format.
 *
 * @param output ostream to pack data into
 *
 * @return output will have text readable data status
 */
bool actiCHampDevice::DataStatus(string& data_status) const
{
    t_champDataStatus champ_data_info;
    int e = champGetDataStatus(device, &champ_data_info);
    if (e != CHAMP_ERR_OK)
    {
        _ACTICHAMP_ERROR_STREAM << "Could not get device data status. actiCHamp Error: " << e << endl;
        return false;
    }
    else
    {
        stringstream ss;
        ss << "Data Status: "      << "\n"
               << "  Total Samples = " << champ_data_info.Samples << "\n"
               << "  Errors = "        << champ_data_info.Errors  << "\n"
               << "  Rate = "          << champ_data_info.Rate    << "\n"
               << "  Speed = "         << champ_data_info.Speed   << "\n"
               << endl;
        data_status = ss.str();
    }
    return true;
}


/**
 * Get the error status of the device into ostream output
 *
 * Gets the device error status, and outputs it in to a ostream in a text
 * readable format.
 *
 * @param output ostream to pack error into
 *
 * @return output will have text readable error status
 */
bool actiCHampDevice::ErrorStatus(string& error_status) const
{
    t_champErrorStatus champ_error_info;
    int e = champGetErrorStatus(device, &champ_error_info);
    if (e != CHAMP_ERR_OK)
    {
        _ACTICHAMP_ERROR_STREAM << "Could not get device error status. actiCHamp Error: " << e << endl;
        return false;
    }
    else
    {
        stringstream ss;
        ss << "Error Status: "      << "\n"
               << "  Total Samples = "  << champ_error_info.Samples    << "\n"
               << "  CRC Errors = "     << champ_error_info.Crc        << "\n"
               << "  Counter Errors = " << champ_error_info.Counter    << "\n"
               << "  Error on Aux = "   << champ_error_info.Modules[0] << "\n"
               << "  Error on M1 = "    << champ_error_info.Modules[1] << "\n"
               << "  Error on M2 = "    << champ_error_info.Modules[2] << "\n"
               << "  Error on M3 = "    << champ_error_info.Modules[3] << "\n"
               << "  Error on M4 = "    << champ_error_info.Modules[4] << "\n"
               << "  Error on M5 = "    << champ_error_info.Modules[5] << "\n"
               << endl;
        error_status = ss.str();
    }
        return true;
}

/**
 * Get the version of the device into ostream output
 *
 * Gets the device version, and outputs it in to a ostream in a text
 * readable format.
 *
 * @param output ostream to pack error into
 *
 * @return output will have text readable version
 */
void actiCHampDevice::DeviceVersion(string& version) const
{
    stringstream ss;
    ss << "Device Version Information: " << "\n"
           << "    Dll: "                    << device_settings.version.Dll     << "\n"
           << "    DRIVER: "                 << device_settings.version.Driver  << "\n"
           << "    CYPRESS: "                << device_settings.version.Cypress << "\n"
           << "    FPGA: "                   << device_settings.version.Fpga    << "\n"
           << "    MSP340: "                 << device_settings.version.Msp430  << "\n" 
           << endl;
    version = ss.str();
}

/**
 * Get the properties of the device into ostream output
 *
 * Gets the device properties, and outputs it in to a ostream in a text
 * readable format.
 *
 * @param output ostream to pack error into
 *
 * @return output will have text readable properties
 */
void actiCHampDevice::GetProperties(string& properties) const
{
    stringstream ss;
    ss << "Device Property Information: "                << "\n"
           << "Number of EEG channels"
           << "numbers of Eeg channels: "                    << device_settings.properties.CountEeg      << "\n"
           << "numbers of Aux channels: "                    << device_settings.properties.CountAux    << "\n"
           << "numbers of input triggers: "                  << device_settings.properties.TriggersIn    << "\n"

           << "numbers of output triggers: "                 << device_settings.properties.TriggersOut   << "\n"
           << "!< Sampling rate, Hz: "                       << device_settings.properties.Rate          << "\n"
           << "!< EEG amplitude scale coefficients, V/bit: " << device_settings.properties.ResolutionEeg <<
    "\n"
           << "!< AUX amplitude scale coefficients, V/bit: " << device_settings.properties.ResolutionAux <<
    "\n"
           << "!< EEG input range peak-peak, V: "            << device_settings.properties.RangeEeg      << "\n"
           << "!< AUX input range peak-peak, V: "            << device_settings.properties.RangeAux      << "\n"
           << endl;
    properties = ss.str();
}

bool actiCHampDevice::set_rate(unsigned int r)
{
    this->device_settings.desired_rate = r;

    switch(device_settings.desired_rate)
    {
        case 100000:
            device_settings.rate = CHAMP_RATE_100KHZ;
            device_settings.averaging = CHAMP_ADC_NATIVE;
            device_settings.decimation = CHAMP_DECIMATION_0;
            return true;
            break;
        case 50000:
            device_settings.rate = CHAMP_RATE_50KHZ;
            device_settings.averaging = CHAMP_ADC_NATIVE;
            device_settings.decimation = CHAMP_DECIMATION_0;
            return true;
            break;
        case 10000:
            device_settings.rate = CHAMP_RATE_10KHZ;
            device_settings.averaging = CHAMP_ADC_NATIVE;
            device_settings.decimation = CHAMP_DECIMATION_0;
            return true;
            break;
        default:
            if(device_settings.desired_rate > 100000)
            {
            _ACTICHAMP_ERROR_STREAM << "Unsupported user entered rate" << endl;
            return false;
            }
            else if(device_settings.desired_rate > 50000)
            {
                if (100000%device_settings.desired_rate != 0)
                {
                    _ACTICHAMP_ERROR_STREAM << "Unsupported user rate, must cleanly go into 10000" << endl;
                    return false;
                }
                else
                {
                    device_settings.rate = CHAMP_RATE_100KHZ;
                    device_settings.averaging = (t_champAdcFilter)((int)100000/device_settings.desired_rate);
                    device_settings.decimation = (t_champDecimation)((int)100000/device_settings.desired_rate);
                    return true;
                }
            }
            else if(device_settings.desired_rate > 10000)
            {
                if (50000%device_settings.desired_rate != 0)
                {
                    _ACTICHAMP_ERROR_STREAM << "Unsupported user rate, must cleanly go into 10000" << endl;
                    return false;
                }
                else
                {
                    device_settings.rate = CHAMP_RATE_50KHZ;
                    device_settings.averaging = (t_champAdcFilter)((int)50000/device_settings.desired_rate);
                    device_settings.decimation = (t_champDecimation)((int)50000/device_settings.desired_rate);
                    return true;
                }
            }
            else if(device_settings.desired_rate > 0)
            {
                if (10000%device_settings.desired_rate != 0)
                {
                    _ACTICHAMP_ERROR_STREAM << "Unsupported user rate, must cleanly go into 10000" << endl;
                    return false;
                }
                else
                {
                    device_settings.rate = CHAMP_RATE_10KHZ;
                    device_settings.averaging = (t_champAdcFilter)((int)50000/device_settings.desired_rate);
                    device_settings.decimation = (t_champDecimation)((int)10000/device_settings.desired_rate);
                    return true;
                }
            }
            break;
    }

    return false;

}

signed int actiCHampDevice::get_channel(t_champDataModelAux& data, unsigned int channel)
{
    if(channel < library_settings.number_of_channels)
            return data.Aux[channel];
    else
        return 0;
}
signed int actiCHampDevice::get_channel(t_champDataModel32& data, unsigned int channel)
{
    if(channel < library_settings.number_of_channels)
        if(channel >= library_settings.number_of_channels -  8)
        {
            return data.Aux[channel];
        }
        else
        {
            return data.Main[channel];
        }
    else
        return 0;
}
signed int actiCHampDevice::get_channel(t_champDataModel64& data, unsigned int channel)
{
    if(channel < library_settings.number_of_channels)
        if(channel >= library_settings.number_of_channels -  8)
        {
            return data.Aux[channel];
        }
        else
        {
            return data.Main[channel];
        }
    else
        return 0;

}
signed int actiCHampDevice::get_channel(t_champDataModel96& data, unsigned int channel)
{
    if(channel < library_settings.number_of_channels)
        if(channel >= library_settings.number_of_channels -  8)
        {
            return data.Aux[channel];
        }
        else
        {
            return data.Main[channel];
        }
    else
        return 0;
}
signed int actiCHampDevice::get_channel(t_champDataModel128& data, unsigned int channel)
{
    if(channel < library_settings.number_of_channels)
        if(channel >= library_settings.number_of_channels -  8)
        {
            return data.Aux[channel];
        }
        else
        {
            return data.Main[channel];
        }
    else
        return 0;
}
signed int actiCHampDevice::get_channel(t_champDataModel160& data, unsigned int channel)
{
    if(channel < library_settings.number_of_channels)
        if(channel >= library_settings.number_of_channels -  8)
        {
            return data.Aux[channel];
        }
        else
        {
            return data.Main[channel];
        }
    else
        return 0;
}



