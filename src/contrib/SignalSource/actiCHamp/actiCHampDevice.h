////////////////////////////////////////////////////////////////////$
// Authors: Paul Ignatenko <paul dot ignatenko at gmail dot com
// Description: Header for device control
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

#include "ActiChamp.h"
#include "GenericSignal.h"
#include <string>
#include <iostream>
#include <ostream>
#include <sstream>

struct DeviceSettings
{
    int               desired_rate;   // User desired sampling rate

    t_champMode       mode;           // Device mode of operation
    t_champRate       rate;           // Device sampling rate
    t_champAdcFilter  averaging;      // Averaging filter values
    t_champDecimation decimation;     // Decimation filter values

    t_champModules    modules;        // Device Modules to use
    t_champVersion    version;        // Version of device
    t_champProperty   properties;     //Properties structure

};

struct LibrarySettings
{
    int init_tries;          // tries device has left to try to load FPGA firmware
    bool debug;              // True for debug output

    bool acquiring_data;     // If we are currently set up to aquire data

    int reference_channel;   // What channel is our reference

    unsigned int number_of_channels;

    HANDLE acquisition_lock; // Mutex handling non-reentrant data collection


    LibrarySettings()  
    {
        init_tries          = 3;
        debug               = false;
        acquiring_data      = false;
        reference_channel   = 0;
        number_of_channels  = 8;
    };

    LibrarySettings & operator= (const LibrarySettings o)
    {
        this->init_tries = o.init_tries;
        this->debug = o.debug;
        this->acquiring_data = o.acquiring_data;
        this->reference_channel = o.reference_channel;

        return *this;
    };
};

class actiCHampDevice {
    public:
        actiCHampDevice();
        ~actiCHampDevice();


        bool open(int device_number = 0 );
        void close();

        bool init();
        bool start();
        bool stop();

        // Data functions
        void get_data (GenericSignal & output, unsigned int size_in_samples);
        void get_data_helper(void* buffer , unsigned int size);

        // Information Funtions
        t_champDataStatus  get_data_status()   const;
        t_champErrorStatus get_error_status()  const;
        t_champVersion     get_version()       const
            { return this->device_settings.version; };
        t_champProperty    get_properties()  const
            { return this->device_settings.properties; };

        bool Settings      ( std::string& settings     ) const;
        bool DataStatus    ( std::string& data_status  ) const;
        bool ErrorStatus   ( std::string& error_status ) const;
        void DeviceVersion ( std::string& version      ) const;
        void GetProperties ( std::string& properties   ) const;

        //Get library_settings related functions
        
        LibrarySettings get_library_settings() const 
            { return this->library_settings; };

        bool get_debug() const
            { return this->library_settings.debug; };

        bool get_acquiring_data() const
            { return this->library_settings.acquiring_data; };


        int get_reference_channel() const
            { return this->library_settings.reference_channel; };



        //Set library_settings related functions
        void set_library_settings(LibrarySettings& s)
            { this->library_settings = s; }; 

        void set_debug(bool d)
            { this->library_settings.debug = d; };


        void set_reference_channel(int r)
            { this->library_settings.reference_channel = r; };


        //Get device_settings related functions
        DeviceSettings get_device_settings() const 
            { return this->device_settings; };

        t_champRate get_rate() const
            {return this->device_settings.rate; };
        t_champMode mode() const
            { return this->device_settings.mode; };

        t_champModules get_modules() const
            { return this->device_settings.modules; };

        t_champDecimation get_decimation() const
            { return this->device_settings.decimation; };

        t_champAdcFilter get_averaging() const
            { return this->device_settings.averaging; };

        //Set device_settings related functions
        void set_device_settings(DeviceSettings s)
            { this->device_settings=s; };

        void  set_mode(t_champMode m)
            { this->device_settings.mode = m; }

        void set_modules(t_champModules m)
            { this->device_settings.modules = m; };

        bool set_rate (unsigned int  r);

        void  set_rate (t_champRate r)
            { this->device_settings.rate = r; };

        void set_decimation (t_champDecimation  d)
            { this->device_settings.decimation = d; };

        void set_averaging  (t_champAdcFilter  a)
            { this->device_settings.averaging = a; };


        //TODO: Make error function useful here? Maybe?
        
    #ifndef ACTICHAMP_TESTING_TOOLKIT
    private:
    #endif
        
        signed int get_channel(t_champDataModelAux& data, unsigned int channel);
        signed int get_channel(t_champDataModel32&  data, unsigned int channel);
        signed int get_channel(t_champDataModel64&  data, unsigned int channel);
        signed int get_channel(t_champDataModel96&  data, unsigned int channel);
        signed int get_channel(t_champDataModel128& data, unsigned int channel);
        signed int get_channel(t_champDataModel160& data, unsigned int channel);

        // Library Variables
        DeviceSettings device_settings;
        LibrarySettings library_settings;

        t_champDataModelAux *dataaux;
        t_champDataModel32 *data32;
        t_champDataModel64 *data64;
        t_champDataModel96 *data96;
        t_champDataModel128 *data128;
        t_champDataModel160 *data160;
        // Hardware Handle
        //
        HANDLE device;

        //Shortcut to apply all settings.
        

};

