/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop

#include "DataGlove5DTUFilter.h"
#include <stdlib.h>
#include <stdio.h>

RegisterFilter( DataGlove5DTUFilter, 3.0 );

// **************************************************************************
// Function:   DataGlove5DTUFilter
// Purpose:    This is the constructor for the DataGlove5DTUFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
DataGlove5DTUFilter::DataGlove5DTUFilter()
{
  BEGIN_PARAMETER_DEFINITIONS
    "HumanInterfaceDevices int DataGloveEnable=             0 0 0 1 "
        "// enable recording from the data glove (0=no, 1=yes) (boolean)",
    "HumanInterfaceDevices int DataGloveHandType=           1 0 0 1"
        "// handedness of the data glove: "
            " 0: left,"
            " 1: right,"
            "(enumeration)",
  END_PARAMETER_DEFINITIONS



  BEGIN_STATE_DEFINITIONS
    "GloveSensor1 12 0 0 0",
    "GloveSensor2 12 0 0 0",
    "GloveSensor3 12 0 0 0",
    "GloveSensor4 12 0 0 0",
    "GloveSensor5 12 0 0 0",
    "GloveSensor6 12 0 0 0",
    "GloveSensor7 12 0 0 0",
    "GloveSensor8 12 0 0 0",
    "GloveSensor9 12 0 0 0",
    "GloveSensor10 12 0 0 0",
    "GloveSensor11 12 0 0 0",
    "GloveSensor12 12 0 0 0",
    "GloveSensor13 12 0 0 0",
    "GloveSensor14 12 0 0 0",
  END_STATE_DEFINITIONS



  hinstLib                = NULL;
  fdOpenCall              = NULL;
  fdCloseCall             = NULL;
  fdScanUSBCall           = NULL;
  fdGetGloveTypeCall      = NULL;
  fdGetGloveHandCall      = NULL;
  fdGetNumSensorsCall     = NULL;
  fdGetSensorRawAllCall   = NULL;
  fdGetSensorRawCall      = NULL;
  pGlove                  = NULL;
  ret                     = -1;
  szPort                  = NULL;
  glovetype               = FD_GLOVENONE;
  glovehand               = -1;
  glovesensors            = -1;
  datagloveenable         = false;

}

// **************************************************************************
// Function:   ~DataGlove5DTUFilter
// Purpose:    This is the destructor for the DataGlove5DTUFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
DataGlove5DTUFilter::~DataGlove5DTUFilter()
{

  if (pGlove != NULL) {
    fdCloseCall(pGlove);
  }

}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void DataGlove5DTUFilter::Preflight( const SignalProperties& Input,
                                           SignalProperties& Output ) const
{

  HINSTANCE                         hinstLib                = NULL;
  FDOPEN                            fdOpenCall              = NULL;
  FDCLOSE                           fdCloseCall             = NULL;
  FDSCANUSB                         fdScanUSBCall           = NULL;
  FDGETGLOVETYPE                    fdGetGloveTypeCall      = NULL;
  FDGETGLOVEHAND                    fdGetGloveHandCall      = NULL;
  FDGETNUMSENSORS                   fdGetNumSensorsCall     = NULL;
  FDGETSENSORRAWALL                 fdGetSensorRawAllCall   = NULL;
  FDGETSENSORRAW                    fdGetSensorRawCall      = NULL;
	fdGlove                          *pGlove                  = NULL;
  int                               ret                     = -1;
	char                             *szPort                  = NULL;
	char	                            szPortToOpen[6];
	int                               glovetype               = FD_GLOVENONE;
	int                               glovehand               = -1;
  int                               glovesensors            = -1;
  bool                              datagloveenable         = false;


  datagloveenable = ( ( int )Parameter( "DataGloveEnable" ) != 0 );

  if (!datagloveenable) {

    hinstLib = NULL;
    hinstLib = LoadLibrary(TEXT("fglove.dll"));

    if (hinstLib != NULL) {

      fdOpenCall               = (FDOPEN)              GetProcAddress(hinstLib, TEXT("?fdOpen@@YAPAUfdGlove@@PAD@Z"));
      fdCloseCall              = (FDCLOSE)             GetProcAddress(hinstLib, TEXT("?fdClose@@YAHPAUfdGlove@@@Z"));
      fdScanUSBCall            = (FDSCANUSB)           GetProcAddress(hinstLib, TEXT("?fdScanUSB@@YAHPAGAAH@Z"));
      fdGetGloveTypeCall       = (FDGETGLOVETYPE)      GetProcAddress(hinstLib, TEXT("?fdGetGloveType@@YAHPAUfdGlove@@@Z"));
      fdGetGloveHandCall       = (FDGETGLOVEHAND)      GetProcAddress(hinstLib, TEXT("?fdGetGloveHand@@YAHPAUfdGlove@@@Z"));
      fdGetNumSensorsCall      = (FDGETNUMSENSORS)     GetProcAddress(hinstLib, TEXT("?fdGetNumSensors@@YAHPAUfdGlove@@@Z"));
      fdGetSensorRawAllCall    = (FDGETSENSORRAWALL)   GetProcAddress(hinstLib, TEXT("?fdGetSensorRawAll@@YAXPAUfdGlove@@PAG@Z"));
      fdGetSensorRawCall       = (FDGETSENSORRAW)      GetProcAddress(hinstLib, TEXT("?fdGetSensorRaw@@YAGPAUfdGlove@@H@Z"));

      if (fdOpenCall            != NULL && fdCloseCall        != NULL && fdScanUSBCall       != NULL ||
          fdGetGloveTypeCall    != NULL && fdGetGloveHandCall != NULL && fdGetNumSensorsCall != NULL ||
          fdGetSensorRawAllCall != NULL && fdGetSensorRawCall != NULL) {

        unsigned short aPID[5];
        int nNumFound = 5;
        int nChosen = 0;
        fdScanUSBCall(aPID,nNumFound);

        if (nNumFound > 0) {

          szPort = "USB";
          strcpy(szPortToOpen,szPort);
          sprintf(szPortToOpen,"USB%i",nChosen);

          pGlove = fdOpenCall(szPortToOpen);

          if (pGlove != NULL) {

            glovetype    = fdGetGloveTypeCall(pGlove);
            glovehand    = fdGetGloveHandCall(pGlove);
            glovesensors = fdGetNumSensorsCall(pGlove);

            int parameter_hand_type = Parameter( "DataGloveHandType" );

            if (glovetype == FD_GLOVE5U_USB || glovetype == FD_GLOVE14U_USB) {

              if (parameter_hand_type == 0) {
                bciout << "Left handed data glove 5DTU found on USB port." << std::endl;
              } // if (parameter_hand_type == 0)

              if (parameter_hand_type == 1) {
                bciout << "Right handed data glove 5DTU found on USB port." << std::endl;
              } // if (parameter_hand_type == 1)

              bciout << "Recording form data glove 5DTU is disabled. To enable recording check DataGloveEnable in section HumanInterface Devices" << std::endl;

            } // if (glovetype == FD_GLOVE5U_USB || glovetype == FD_GLOVE14U_USB)
          } // if (pGlove != NULL)
        } //if (nNumFound > 0)
      } // if call == NULL
    } // (hinstLib != NULL)


  } else {

    hinstLib = NULL;
    hinstLib = LoadLibrary(TEXT("fglove.dll"));

    if (hinstLib == NULL) {
      bcierr << "Dynamic Link Library fglove.dll for data glove 5DTU not found" << std::endl;
      return;
    } else {
     fdOpenCall               = (FDOPEN)              GetProcAddress(hinstLib, TEXT("?fdOpen@@YAPAUfdGlove@@PAD@Z"));
     fdCloseCall              = (FDCLOSE)             GetProcAddress(hinstLib, TEXT("?fdClose@@YAHPAUfdGlove@@@Z"));
     fdScanUSBCall            = (FDSCANUSB)           GetProcAddress(hinstLib, TEXT("?fdScanUSB@@YAHPAGAAH@Z"));
     fdGetGloveTypeCall       = (FDGETGLOVETYPE)      GetProcAddress(hinstLib, TEXT("?fdGetGloveType@@YAHPAUfdGlove@@@Z"));
     fdGetGloveHandCall       = (FDGETGLOVEHAND)      GetProcAddress(hinstLib, TEXT("?fdGetGloveHand@@YAHPAUfdGlove@@@Z"));
     fdGetNumSensorsCall      = (FDGETNUMSENSORS)     GetProcAddress(hinstLib, TEXT("?fdGetNumSensors@@YAHPAUfdGlove@@@Z"));
     fdGetSensorRawAllCall    = (FDGETSENSORRAWALL)   GetProcAddress(hinstLib, TEXT("?fdGetSensorRawAll@@YAXPAUfdGlove@@PAG@Z"));
     fdGetSensorRawCall       = (FDGETSENSORRAW)      GetProcAddress(hinstLib, TEXT("?fdGetSensorRaw@@YAGPAUfdGlove@@H@Z"));
    }

    if (fdOpenCall            == NULL || fdCloseCall        == NULL || fdScanUSBCall       == NULL ||
        fdGetGloveTypeCall    == NULL || fdGetGloveHandCall == NULL || fdGetNumSensorsCall == NULL ||
        fdGetSensorRawAllCall == NULL || fdGetSensorRawCall == NULL) {

        bcierr << "Invalid version of fglove.dll" << std::endl;
        return;
    }


    if (fdOpenCall != NULL) {


      unsigned short aPID[5];
      int nNumFound = 5;
      int nChosen = 0;
      fdScanUSBCall(aPID,nNumFound);

      if (nNumFound == 0) {
        bcierr << "No data glove 5DTU found on USB port" << std::endl;
        return;
      }

      if (nNumFound > 1) {
        bciout << "More than one data glove 5DTU found on USB port, will proceed with first found" << std::endl;
      }

      szPort = "USB";
      strcpy(szPortToOpen,szPort);
      sprintf(szPortToOpen,"USB%i",nChosen);

      pGlove = fdOpenCall(szPortToOpen);

      if (pGlove == NULL) {

        bcierr << "Data Glove 5DTU is not connected to the USB port" << std::endl;
        return;

      } else {
        glovetype    = fdGetGloveTypeCall(pGlove);
        glovehand    = fdGetGloveHandCall(pGlove);
        glovesensors = fdGetNumSensorsCall(pGlove);


        if (glovetype != FD_GLOVE5U_USB && glovetype != FD_GLOVE14U_USB) {
          bcierr << "Wrong type of data glove is connected to the USB port" << std::endl;
          return;
        }

        int parameter_hand_type = Parameter( "DataGloveHandType" );

        if (glovehand == FD_HAND_LEFT && parameter_hand_type != 0) {
          bcierr << "Mismatch between left handed data glove 5DTU found on USB port and right handed selected in DataGloveHandType in section HumanInterfaceDevices" << std::endl;
          return;
        }

        if (glovehand == FD_HAND_RIGHT && parameter_hand_type != 1) {
          bcierr << "Mismatch between right handed data glove 5DTU found on USB port and left handed selected in DataGloveHandType in section HumanInterfaceDevices" << std::endl;
          return;
        }


      }

    }
  }


  Output = Input;

}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the DataGlove5DTUFilter
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void DataGlove5DTUFilter::Initialize( const SignalProperties&, const SignalProperties& )
{

  datagloveenable = ( ( int )Parameter( "DataGloveEnable" ) != 0 );

  if (!datagloveenable) {

    hinstLib = NULL;
    hinstLib = LoadLibrary(TEXT("fglove.dll"));

    if (hinstLib != NULL) {

      fdOpenCall               = (FDOPEN)              GetProcAddress(hinstLib, TEXT("?fdOpen@@YAPAUfdGlove@@PAD@Z"));
      fdCloseCall              = (FDCLOSE)             GetProcAddress(hinstLib, TEXT("?fdClose@@YAHPAUfdGlove@@@Z"));
      fdScanUSBCall            = (FDSCANUSB)           GetProcAddress(hinstLib, TEXT("?fdScanUSB@@YAHPAGAAH@Z"));
      fdGetGloveTypeCall       = (FDGETGLOVETYPE)      GetProcAddress(hinstLib, TEXT("?fdGetGloveType@@YAHPAUfdGlove@@@Z"));
      fdGetGloveHandCall       = (FDGETGLOVEHAND)      GetProcAddress(hinstLib, TEXT("?fdGetGloveHand@@YAHPAUfdGlove@@@Z"));
      fdGetNumSensorsCall      = (FDGETNUMSENSORS)     GetProcAddress(hinstLib, TEXT("?fdGetNumSensors@@YAHPAUfdGlove@@@Z"));
      fdGetSensorRawAllCall    = (FDGETSENSORRAWALL)   GetProcAddress(hinstLib, TEXT("?fdGetSensorRawAll@@YAXPAUfdGlove@@PAG@Z"));
      fdGetSensorRawCall       = (FDGETSENSORRAW)      GetProcAddress(hinstLib, TEXT("?fdGetSensorRaw@@YAGPAUfdGlove@@H@Z"));

      if (fdOpenCall            != NULL && fdCloseCall        != NULL && fdScanUSBCall       != NULL ||
          fdGetGloveTypeCall    != NULL && fdGetGloveHandCall != NULL && fdGetNumSensorsCall != NULL ||
          fdGetSensorRawAllCall != NULL && fdGetSensorRawCall != NULL) {

        unsigned short aPID[5];
        int nNumFound = 5;
        int nChosen = 0;
        fdScanUSBCall(aPID,nNumFound);

        if (nNumFound > 0) {

          szPort = "USB";
          strcpy(szPortToOpen,szPort);
          sprintf(szPortToOpen,"USB%i",nChosen);

          pGlove = fdOpenCall(szPortToOpen);

          if (pGlove != NULL) {

            glovetype    = fdGetGloveTypeCall(pGlove);
            glovehand    = fdGetGloveHandCall(pGlove);
            glovesensors = fdGetNumSensorsCall(pGlove);

            int parameter_hand_type = Parameter( "DataGloveHandType" );

            if (glovetype == FD_GLOVE5U_USB || glovetype == FD_GLOVE14U_USB) {

              if (parameter_hand_type == 0) {
                bciout << "Left handed data glove 5DTU found on USB port." << std::endl;
              } // if (parameter_hand_type == 0)

              if (parameter_hand_type == 1) {
                bciout << "Right handed data glove 5DTU found on USB port." << std::endl;
              } // if (parameter_hand_type == 1)

              bciout << "Recording form data glove 5DTU is disabled. To enable recording check DataGloveEnable in section HumanInterface Devices" << std::endl;

            } // if (glovetype == FD_GLOVE5U_USB || glovetype == FD_GLOVE14U_US)
          } // if (pGlove != NULL)
        } //if (nNumFound > 0)
      } // if call == NULL
    } // (hinstLib != NULL)


  } else {

    hinstLib = NULL;
    hinstLib = LoadLibrary(TEXT("fglove.dll"));

    if (hinstLib == NULL) {
      bcierr << "Dynamic Link Library fglove.dll for data glove 5DTU not found" << std::endl;
      return;
    } else {
     fdOpenCall               = (FDOPEN)              GetProcAddress(hinstLib, TEXT("?fdOpen@@YAPAUfdGlove@@PAD@Z"));
     fdCloseCall              = (FDCLOSE)             GetProcAddress(hinstLib, TEXT("?fdClose@@YAHPAUfdGlove@@@Z"));
     fdScanUSBCall            = (FDSCANUSB)           GetProcAddress(hinstLib, TEXT("?fdScanUSB@@YAHPAGAAH@Z"));
     fdGetGloveTypeCall       = (FDGETGLOVETYPE)      GetProcAddress(hinstLib, TEXT("?fdGetGloveType@@YAHPAUfdGlove@@@Z"));
     fdGetGloveHandCall       = (FDGETGLOVEHAND)      GetProcAddress(hinstLib, TEXT("?fdGetGloveHand@@YAHPAUfdGlove@@@Z"));
     fdGetNumSensorsCall      = (FDGETNUMSENSORS)     GetProcAddress(hinstLib, TEXT("?fdGetNumSensors@@YAHPAUfdGlove@@@Z"));
     fdGetSensorRawAllCall    = (FDGETSENSORRAWALL)   GetProcAddress(hinstLib, TEXT("?fdGetSensorRawAll@@YAXPAUfdGlove@@PAG@Z"));
     fdGetSensorRawCall       = (FDGETSENSORRAW)      GetProcAddress(hinstLib, TEXT("?fdGetSensorRaw@@YAGPAUfdGlove@@H@Z"));
    }

    if (fdOpenCall            == NULL || fdCloseCall        == NULL || fdScanUSBCall       == NULL ||
        fdGetGloveTypeCall    == NULL || fdGetGloveHandCall == NULL || fdGetNumSensorsCall == NULL ||
        fdGetSensorRawAllCall == NULL || fdGetSensorRawCall == NULL) {

        bcierr << "Invalid version of fglove.dll" << std::endl;
        return;
    }


    if (fdOpenCall != NULL) {


      unsigned short aPID[5];
      int nNumFound = 5;
      int nChosen = 0;
      fdScanUSBCall(aPID,nNumFound);

      if (nNumFound == 0) {
        bcierr << "No data glove 5DTU found on USB port" << std::endl;
        return;
      }

      if (nNumFound > 1) {
        bciout << "More than one data glove 5DTU found on USB port, will proceed with first found" << std::endl;
        return;
      }

      szPort = "USB";
      strcpy(szPortToOpen,szPort);
      sprintf(szPortToOpen,"USB%i",nChosen);

      pGlove = fdOpenCall(szPortToOpen);

      if (pGlove == NULL) {

        bcierr << "Data Glove 5DTU is not connected to the USB port" << std::endl;
        return;

      } else {
        glovetype    = fdGetGloveTypeCall(pGlove);
        glovehand    = fdGetGloveHandCall(pGlove);
        glovesensors = fdGetNumSensorsCall(pGlove);


        if (glovetype != FD_GLOVE5U_USB && glovetype != FD_GLOVE14U_USB) {
          bcierr << "Wrong type of data glove is connected to the USB port" << std::endl;
          return;
        }

        int parameter_hand_type = Parameter( "DataGloveHandType" );

        if (glovehand == FD_HAND_LEFT && parameter_hand_type != 0) {
          bcierr << "Mismatch between left handed data glove 5DTU found on USB port and right handed selected in DataGloveHandType" << std::endl;
          return;
        }

        if (glovehand == FD_HAND_RIGHT && parameter_hand_type != 1) {
          bcierr << "Mismatch between right handed data glove 5DTU found on USB port and left handed selected in DataGloveHandType" << std::endl;
          return;
        }


      }

    }
  }

}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the calibration routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************
void DataGlove5DTUFilter::Process(const GenericSignal& Input, GenericSignal& Output)
{

  if (datagloveenable) {

    sensor_data[0]  = fdGetSensorRawCall(pGlove,FD_THUMBNEAR);
    sensor_data[1]  = fdGetSensorRawCall(pGlove,FD_INDEXNEAR);
    sensor_data[2]  = fdGetSensorRawCall(pGlove,FD_MIDDLENEAR);
    sensor_data[3]  = fdGetSensorRawCall(pGlove,FD_RINGNEAR);
    sensor_data[4]  = fdGetSensorRawCall(pGlove,FD_LITTLENEAR);

    if (glovetype == FD_GLOVE14U_USB) {
      sensor_data[5]  = fdGetSensorRawCall(pGlove,FD_THUMBFAR);
      sensor_data[6]  = fdGetSensorRawCall(pGlove,FD_INDEXFAR);
      sensor_data[7]  = fdGetSensorRawCall(pGlove,FD_MIDDLEFAR);
      sensor_data[8]  = fdGetSensorRawCall(pGlove,FD_RINGFAR);
      sensor_data[9]  = fdGetSensorRawCall(pGlove,FD_LITTLEFAR);
      sensor_data[10] = fdGetSensorRawCall(pGlove,FD_THUMBINDEX);
      sensor_data[11] = fdGetSensorRawCall(pGlove,FD_INDEXMIDDLE);
      sensor_data[12] = fdGetSensorRawCall(pGlove,FD_MIDDLERING);
      sensor_data[13] = fdGetSensorRawCall(pGlove,FD_RINGLITTLE);
    } else {
      sensor_data[5]  = 0;
      sensor_data[6]  = 0;
      sensor_data[7]  = 0;
      sensor_data[8]  = 0;
      sensor_data[9]  = 0;
      sensor_data[10] = 0;
      sensor_data[11] = 0;
      sensor_data[12] = 0;
      sensor_data[13] = 0;
    }

    State("GloveSensor1")=sensor_data[0];
    State("GloveSensor2")=sensor_data[1];
    State("GloveSensor3")=sensor_data[2];
    State("GloveSensor4")=sensor_data[3];
    State("GloveSensor5")=sensor_data[4];
    State("GloveSensor6")=sensor_data[5];
    State("GloveSensor7")=sensor_data[6];
    State("GloveSensor8")=sensor_data[7];
    State("GloveSensor9")=sensor_data[8];
    State("GloveSensor10")=sensor_data[9];
    State("GloveSensor11")=sensor_data[10];
    State("GloveSensor12")=sensor_data[11];
    State("GloveSensor13")=sensor_data[12];
    State("GloveSensor14")=sensor_data[13];

  } else {

    State("GloveSensor1")=0;
    State("GloveSensor2")=0;
    State("GloveSensor3")=0;
    State("GloveSensor4")=0;
    State("GloveSensor5")=0;
    State("GloveSensor6")=0;
    State("GloveSensor7")=0;
    State("GloveSensor8")=0;
    State("GloveSensor9")=0;
    State("GloveSensor10")=0;
    State("GloveSensor11")=0;
    State("GloveSensor12")=0;
    State("GloveSensor13")=0;
    State("GloveSensor14")=0;
  }

   Output = Input;

}



