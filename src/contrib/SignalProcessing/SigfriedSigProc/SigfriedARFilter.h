/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#define WIN32

#ifndef SigfriedARFilterH
#define SigfriedARFilterH
//---------------------------------------------------------------------------


#include "defines.h"
#include "vector.h"

#include "GenericFilter.h"
#include "GenericVisualization.h"

#include "array.h"
#include "namespace.h"
#include "exception.h"

#include "ElectrodeRenderer.h"
#include "ElectrodeCollection.h"

typedef struct ELECTRODEDESCRIPTION {
  string szlabel;
  float  posx;
  float  posy;
} TELECTRODEDESCRIPTION;

typedef struct CONDITIONELEMENT {
  string szstate;
  string szoperator;
  int    value;
} TCONDITIONELEMENT;

typedef struct CONDITION {
  string szname;
  int    reference;
  vector< TCONDITIONELEMENT > conditionelements;
  float  dispMin;
  float  dispMax;
} TCONDITION;

typedef struct POSITION {
  int Left;
  int Top;
  int Width;
} TPOSITION;


// Multi-Threading: Not yet finished!
/*
class SigfriedARFilter;
typedef struct WORKERTHREAD {
  SigfriedARFilter *pOwner;
  unsigned int      identity;
  int               counter;
} TWORKERTHREAD;
*/

class TSigfriedForm;

///////////////////////////////////////////////////////////////////////////////
/// SigfriedARFilter class
/// @author <a href="mailto:pbrunner@wadsworth.org">Brunner Peter</a>
/// @version 1.0
/// @brief This class manages a number of instances of #CElectrodeCollection
/// and renders them to the device context passed with the constructor
/// with an given offset.
///////////////////////////////////////////////////////////////////////////////
class SigfriedARFilter : public GenericFilter
{
private:


  enum TScoreType
  {
      mahalanobis_distance = 0,
      neg_log_probability  = 1,
  };

  enum TFeedbackType
  {
      log_score            = 0,
      real_time_display    = 1,
  };

  enum
  {
  #undef MAX_N
   MAX_N = 256,
  #undef MAX_M
   MAX_M = 256,
  };

  /// Variable for parameter VisualizeSigfried (either 1 or 0).
  int                                       VisualizeSigfried;

  /// Variable for parameter SampleBlockSize.
  int                                       SampleBlockSize;

  /// Variable for parameter MemWindows.
  int                                       MemWindows;

  /// Variable for parameter SamplingRate.
  int                                       SamplingRate;

  /// Variable for the parameter LearningRateHist.
  float                                     LearningRateHist;

  /// Variable for the parameter StatisticDisplayType.
  int                                       StatisticDisplayType;

  /// Variable for the parameter LearningRateAverage.
  float                                     LearningRateAverage;

  /// Variable for the parameter LearningRateAutoScale.
  float                                     LearningRateAutoScale;

  /// Variable for the parameter CircleRadius.
  int                                       CircleRadius;

  /// Variable for the parameter ScoreType.
  int                                       ScoreType;

  /// Variable that equals datawindows*samples.
  int                                       winlgth;

  /// Total time spend in the #Process funtion.
  double                                    totaltimesum;

  /// Number of times the #Process function was called.
  long                                      counter;

  /// Marker set when SIGFRIED visualization is initailized.
  bool                                      visualize;

  /// Screen Width (covering all monitors).
  int                                       ScreenWidth;

  /// Screen Height (covering all monitors).
  int                                       ScreenHeight;

  /// Screen Left Start Position (covering all monitors).
  int                                       ScreenLeft;

  /// Screen Top Start Position (covering all monitors).
  int                                       ScreenTop;

  /// Vector containing #num_models identities of the GenericVisualization.
  /// Used to display the SIGFRIED score for each model.
  vector <GenericVisualization *>           vscoreVis;

  /// Vector containing #num_models identities of a vector of checkboxes for
  /// each condition.
  vector< vector<TCheckBox *> >             vpCheckBox;

  /// Vector containing #num_models identities of a vector of boolean markers
  /// for each condition to indicate if they should be rendered.
  /// Variable is mirroring the #vpCheckBox.
  vector< vector<bool> >                    vbRender;

  /// Variable for the parameter SourceChGain.
  /// Defines how many microVolts is one A/D unit.
  vector< float >                           vSourceChGain;

  /// Variable for the parameter SourceChOffset.
  /// Defines how many A/D units is the offset.
  vector< float >                           vSourceChOffset;

  /// Variable for the parameter AutoScaleChannelList.
  /// Defines which channels are used for the autoscale features.
  vector< int >                             vAutoScaleChannelList;

  /// Stores the current form with for each identity of #vpForm.
  vector <int>                              vform_width;

  /// Stores the current form height for each identity of #vpForm.
  vector <int>                              vform_height;

  /// Stores the description in the parameter ElectrodeLocation.
  vector< TELECTRODEDESCRIPTION >           electrodesdescription;

  /// Stores the conditions in the parameter ElectrodeCondition.
  vector< TCONDITION >                      conditions;

  /// Vector with #num_models identities of the a vector of
  /// electrode collections for each condition.
  vector< vector< CElectrodeCollection* > > velectrodecollections;

  /// Vector with #num_models identities of renderes to render
  /// the SIGFRIED display for each model.
  vector< CElectrodeRenderer* >             vpElectrodeRenderer;

  /// Stores the position of each idendity of #vpForm.
  vector< TPOSITION >                       vform_position;

  /// Stores the first column of the parameter ModelFiles.
  /// Represents the relative/full path to the model file. 
  vector< const char * >                    vmodel_filename;

  /// Stores the second column of the parameter ModelFiles.
  /// Represents the caption of the idendity of #vpForm.
  vector< const char * >                    vmodel_label;

  /// Vector with #num_models identities of the #TSigfriedForm.
  vector< TSigfriedForm * >                 vpForm;

  /// Vector with #num_modles identities of the model configuration.
  vector< SIGFRIED_PARAMETERS_CFG >         vparameters_cfg;

  /// Vector with #num_models identities of the model command line parameters.
  vector< DATA2MODEL_PARAMETERS_CMD >       vparameters_cmd;

  /// Vector with #num_models identities of the model parameters.
  vector< SIGFRIEDmodeller_PARAMETERS >     vparameters_sig;

  /// Vector with #num_models identities of the features that are extracted
  /// from the signal in each call of #Process.
  vector< CVector<float> >                  vfeaturesamples;

  /// Vector with #num_models identities of the features that are extracted
  /// from the signal and normalized to the range of the corresponding model.
  vector< CVector<float> >                  vfeaturesamples_out;

  /// Vector with #num_models identities of the model.
  vector< CVector<SIGFRIED_MODEL_TYPE> >    vmodel;

  /// Vector with #num_models identities of the min featuresample value
  /// for each channel in the model.
  vector< CVector<float> >                  vtrain_min;
  
  /// Vector with #num_models identities of the max featuresample value
  /// for each channel in the model.
  vector< CVector<float> >                  vtrain_max;

  /// Vector with #num_models identities of the score that is calculated
  /// for each channel in every call of #Process.
  vector< CVector<float> >                  vscore;

  /// Vector with #num_models identities of the score that is retrieved
  /// from the real-time display.
  vector< CVector<float> >                  vscore_display;

  /// Vector with #num_models identities of the first-in-first-out buffer
  /// that is filled in every call of #Process.
  CVector< float >                          vsignal;

  /// Number of columns in the parameter ElectrodeCondition.
  /// Represents the number of condition elements. 
  int                                       num_electrodecondition_columns;

  /// Number of rows in the parameter ElectrodeCondition.
  /// Must be fullfill: num_electrodecondition_columns-4) % 3 == 0
  /// to be a valid condition.
  int                                       num_electrodecondition_rows;

  /// Number of columns in the parameter ElectrodeLocation.
  /// Must be exactly 3 to be a valid entry.
  int                                       num_electrodelocation_columns;

  /// Number of rows in the parameter ElectrodeLocation.
  /// Represents the number of electrodes.
  /// Must be equal to the number of channels in the Parameter TransmitChList.
  int                                       num_electrodelocation_rows;

  /// Number of models to be scored and visualized.
  unsigned int                              num_models;

  /// Minimum x-coordinate in #electrodesdescription.
  float                                     electrodeposxmin;

  /// Maximum x-coordinate in #electrodesdescription.
  float                                     electrodeposxmax;

  /// Minimum y-coordinate in #electrodesdescription.
  float                                     electrodeposymin;

  /// Maximum y-coordinate in #electrodesdescription.
  float                                     electrodeposymax;

  /// Calculated aspect ratio from
  /// (electrodeposymax - electrodeposymin) / (electrodeposxmax - electrodeposxmin)
  float                                     electrodeaspectratio;

  /// Marker that indicates that everything is intialized.
  /// Marker is set in the function #Initialize.
  bool                                      binitialized;

  /// Marker that indicates that the form is currently being resized
  /// Marker is set in the function #FormResize.
  bool                                      bresizing;

  /// Marker that indicates that the user has started processing by
  /// klicking on the "Start" button.
  /// Marker is set in #StartRun function.
  /// Marker is unset in #StopRun function.
  bool                                      bprocessing;

  /// Marker that indicates that the user has clicked on one of the
  /// checkboxes.
  bool                                      bclicked;

  /// First-in-first-out buffer that stores the signal.
  float                                     datwin[MAX_M][MAX_N*8];

  /// Buffer variable that holds the size of the features.
  int                                       feature_size[3];

  /// Number of channels in the model.
  /// Must match the number of channels in the parameter TransmitChList.
  int                                       num_channels;

  /// Holds the minimum distance between two electrodes.
  float                                     min_elect_dist;

  /// Variable for the parameter FeedbackType.
  int                                       feedback_type;

  /// Handle to the SIGFRIED.dll
  HINSTANCE                                 hinstLib;

  /// Handle to the set_std_out function in the SIGFRIED.dll.
  SET_STD_OUT                               set_std_out;

  /// Handle to the set_malloc function in the SIGFRIED.dll.
  SET_MALLOC                                set_malloc;

  /// Handle to the set_free function in the SIGFRIED.dll.
  SET_FREE                                  set_free;

  /// Handle to the load_model function in the SIGFRIED.dll.
  LOAD_MODEL                                load_model;

  /// Handle to the get_feature_size_sample function in the SIGFRIED.dll.
  GET_FEATURE_SIZE_SAMPLE                   get_feature_size_sample;

  /// Handle to the build_data_sample_init function in the SIGFRIED.dll.
  BUILD_DATA_FREQUENCY_SAMPLE_INIT          build_data_sample_init;

  /// Handle to the build_data_sample function in the SIGFRIED.dll.
  BUILD_DATA_FREQUENCY_SAMPLE               build_data_sample;

  /// Handle to the build_data function in the SIGFRIED.dll.
  BUILD_DATA_FREQUENCY                      build_data;

  /// Handle to the normalize_data function in the SIGFRIED.dll.
  NORMALIZE_DATA                            normalize_data;

  /// Handle to the score_data function in the SIGFRIED.dll.
  SCORE_DATA                                score_data;

  void CreateDisplay();
  void DestroyDisplay();


public:
  SigfriedARFilter();
  virtual ~SigfriedARFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process(const GenericSignal& Input, GenericSignal& Output);
  virtual void StartRun();
  virtual void StopRun();

  void __fastcall FormResize(TObject *Sender);
  void __fastcall FormShow  (TObject *Sender);
  void __fastcall ClickedBox(TObject *Sender);

  bool IsRunning() { return bprocessing; };


// Multi-Threading: Not yet finished!
/*
  void ProcessWorkerThread(int index_model);
  static DWORD WINAPI TestThread(LPVOID param);
  HANDLE thread[16];
  HANDLE eventworkerstart[16];
  HANDLE eventworkerfinished[16];
  TWORKERTHREAD workerthread[16];
*/

};

// create a form that will contain the SIGFRIED display
class TSigfriedForm : public TForm
{
 public:
  TSigfriedForm() : TForm( static_cast<TComponent*>(NULL), 1 )
    {
    }

  // lock the WMSyscommand message in case of a running system.
  void __fastcall WMSyscommand (TWMSysCommand& command)
    {

      if( pOwner->IsRunning() ){
        Sleep(0);
      } else {
        Sleep(0);
        TForm::Dispatch(&command);
      }

    }

  BEGIN_MESSAGE_MAP
      VCL_MESSAGE_HANDLER(WM_SYSCOMMAND, TWMSysCommand, WMSyscommand)
  END_MESSAGE_MAP( TForm )

  SigfriedARFilter *pOwner;
  HWND hWND;

};


#endif

