#ifndef GENERIC_DEFINES
#define GENERIC_DEFINES

#define ERR_NOERR               0
#define ERR_STATENOTFOUND       1
#define ERR_SOURCENOTCONNECTED  2

#define COREMODULE_EEGSOURCE    1
#define COREMODULE_SIGPROC      2
#define COREMODULE_APPLICATION  3
#define COREMODULE_OPERATOR     4

#define PARAMNAME_EEGSOURCEIP           "EEGsourceIP"
#define PARAMNAME_EEGSOURCEPORT         "EEGsourcePort"
#define PARAMNAME_APPLICATIONIP         "ApplicationIP"
#define PARAMNAME_APPLICATIONPORT       "ApplicationPort"
#define PARAMNAME_SIGPROCIP             "SignalProcessingIP"
#define PARAMNAME_SIGPROCPORT           "SignalProcessingPort"

#define KEY_BCI2000             "\\SOFTWARE\\BCI2000"
#define KEY_OPERATOR            "\\OPERATOR"
#define KEY_EEGSOURCE           "\\EEGSOURCE"
#define KEY_SIGPROC             "\\SIGPROC"
#define KEY_APPLICATION         "\\APPLICATION"
#define KEY_VISUALIZATION       "\\VISUALIZATION"
#define KEY_PARAMETERS          "\\PARAMETERS"
#define KEY_CONFIG              "\\CONFIG"

#define VISTYPE_GRAPH           1       // visualization type 1   ... graph t channels x s samples
#define VISTYPE_MEMO            2       // visualization type 2   ... memo field
#define VISTYPE_VISCFG          255     // visualization type 255 ... visualization config

#define CFGID_WINDOWTITLE       1
#define CFGID_MINVALUE          2
#define CFGID_MAXVALUE          3
#define CFGID_NUMSAMPLES        4
#define CFGID_XAXISLABEL        5
#define CFGID_YAXISLABEL        6

#define SOURCEID_EEGDISP        53
#define SOURCEID_CALIBRATION    54
#define SOURCEID_SPATFILT       55
#define SOURCEID_TEMPORALFILT   56
#define SOURCEID_CLASSIFIER     57
#define SOURCEID_NORMALIZER     58
#define SOURCEID_STATISTICS     59
#define SOURCEID_TASKLOG        60
#define SOURCEID_ROUNDTRIP      61
#define SOURCEID_SPELLERTRIALSEQ 62
        
#define DATATYPE_INTEGER        0
#define DATATYPE_FLOAT          1

#define WINDOW_OPEN      (WM_APP + 400)
#define RESET_OPERATOR   (WM_APP + 401)
#define HANDLE_MESSAGE   (WM_APP + 402)
#define STARTDAQ_MESSAGE (WM_APP + 403)
#define RESET_MESSAGE    (WM_APP + 404)
#define WINDOW_RENDER    (WM_APP + 405)

#endif
