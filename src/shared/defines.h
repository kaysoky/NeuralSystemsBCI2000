#ifndef DefinesH
#define DefinesH

#define ERR_NOERR               0
#define ERR_STATENOTFOUND       1
#define ERR_SOURCENOTCONNECTED  2

#define COREMODULE_EEGSOURCE    1
#define COREMODULE_SIGPROC      2
#define COREMODULE_APPLICATION  3
#define COREMODULE_OPERATOR     4

#define OPERATORPORT_EEGSOURCE   4000
#define OPERATORPORT_SIGPROC     4001
#define OPERATORPORT_APPLICATION 4002

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

#define _OLD_NAMES

namespace VISTYPE
{
  enum VISTYPE
  {
    GRAPH = 1,      // visualization type 1   ... graph t channels x s samples
    MEMO = 2,       // visualization type 2   ... memo field
    VISCFG = 255,   // visualization type 255 ... visualization config
  };
};
// Backward compatibility.
#ifdef _OLD_NAMES
# define VISTYPE_GRAPH           (VISTYPE::GRAPH)
# define VISTYPE_MEMO            (VISTYPE::MEMO)
# define VISTYPE_VISCFG          (VISTYPE::VISCFG)
#endif // _OLD_NAMES

namespace CFGID
{
  enum CFGID
  {
    WINDOWTITLE = 1,

      // Graph options
      MINVALUE,
      MAXVALUE,
      NUMSAMPLES,
      XAXISLABEL,
      YAXISLABEL,
      channelGroupSize,
      graphType,
        // Graph types
        polyline,
          // Polyline options
          showBaselines,
        colorfield,

      // Memo options
      numLines,
  };
};
// Backward compatibility.
#ifdef _OLD_NAMES
# define CFGID_WINDOWTITLE (CFGID::WINDOWTITLE)
# define CFGID_MINVALUE    (CFGID::MINVALUE)
# define CFGID_MAXVALUE    (CFGID::MAXVALUE)
# define CFGID_NUMSAMPLES  (CFGID::NUMSAMPLES)
# define CFGID_XAXISLABEL  (CFGID::XAXISLABEL)
# define CFGID_YAXISLABEL  (CFGID::YAXISLABEL)
#endif // _OLD_NAMES

namespace SOURCEID
{
  enum SOURCEID
  {
    EEGDISP = 53,
    CALIBRATION,
    SPATFILT,
    TEMPORALFILT,
    CLASSIFIER,
    NORMALIZER,
    STATISTICS,
    TASKLOG,
    ROUNDTRIP,
    SPELLERTRIALSEQ,
    Average,
    FFT = Average + 10,
    SW,
    Baseline,
    Artefact,
  };
};
// Backward compatibility.
#ifdef _OLD_NAMES
# define SOURCEID_EEGDISP         (SOURCEID::EEGDISP)
# define SOURCEID_CALIBRATION     (SOURCEID::CALIBRATION)
# define SOURCEID_SPATFILT        (SOURCEID::SPATFILT)
# define SOURCEID_TEMPORALFILT    (SOURCEID::TEMPORALFILT)
# define SOURCEID_CLASSIFIER      (SOURCEID::CLASSIFIER)
# define SOURCEID_NORMALIZER      (SOURCEID::NORMALIZER)
# define SOURCEID_STATISTICS      (SOURCEID::STATISTICS)
# define SOURCEID_TASKLOG         (SOURCEID::TASKLOG)
# define SOURCEID_ROUNDTRIP       (SOURCEID::ROUNDTRIP)
# define SOURCEID_SPELLERTRIALSEQ (SOURCEID::SPELLERTRIALSEQ)
#endif // _OLD_NAMES

namespace DATATYPE
{
  enum
  {
    INTEGER = 0,
    FLOAT,
  };
};

// Backward compatibility.
#ifdef _OLD_NAMES
# define DATATYPE_INTEGER        (DATATYPE::INTEGER)
# define DATATYPE_FLOAT          (DATATYPE::FLOAT)
#endif // _OLD_NAMES

#undef _OLD_NAMES

#define RESET_OPERATOR   (WM_APP + 401)
#define HANDLE_MESSAGE   (WM_APP + 402)
#define STARTDAQ_MESSAGE (WM_APP + 403)
#define RESET_MESSAGE    (WM_APP + 404)

#endif // DefinesH
