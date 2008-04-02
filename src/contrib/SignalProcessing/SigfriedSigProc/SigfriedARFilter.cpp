/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include "PCHIncludes.h"
#pragma hdrstop

#include <math.h>
#include <stdio.h>

#include "SigfriedARFilter.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


RegisterFilter( SigfriedARFilter, 2.C );


double LOG(double x) {
  if (x < 1e-30)
    return -69.0776;
  else
    return log(x);
}


// **************************************************************************
// Function:   SigfriedARFilter
// Purpose:    This is the constructor for the SigfriedARFilter class
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
SigfriedARFilter::SigfriedARFilter()
//: mScoreVis( SourceID::EMALG )
{

//  "Filtering string ModelFile= model.mat model.mat a z "
//      "// file that contains the signal model",




 BEGIN_PARAMETER_DEFINITIONS
  "Filtering matrix ModelFiles= "
      "{ 1 2 3} " // row labels
      "{ filename label x-pos y-pos width} " // column labels
      "../model/baseline.mdl	mu/beta 0 0 500 "
      "../model/baseline.mdl  gamma	500 0 500 "
      "../model/baseline.mdl  highgamma	1000 0 500 "
      " // model files ",
  "Visualize int VisualizeSigfried= 1 0 0 1 "
      "// visualize SIGFRIED signals (0=no 1=yes)",
  "Filtering int MemWindows= 2 2 1 1024 "
      "// AR- number of input blocks",
  "Filtering matrix ElectrodeLocation= "
      "{ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 } " // row labels
      "{ label x y } " // column labels
      "Fp1.	-12546	38613	"
      "Fp2.	12546	38613	"
      "F3..	-15773	22527	"
      "F4..	15773	22527	"
      "C3..	-20300	00000	"
      "C4..	20300	00000	"
      "P3..	-15773	-22527	"
      "P4..	15773	-22527	"
      "T7..	-40600	00000	"
      "T8..	40600	00000	"
      "P7..	-32846	-23864	"
      "P8..	32846 -23864	"
      "Fz..	00000  20300	"
      "Pz..	00000 -18100	"
      "Cz..	00000  00000	"
      "Oz..	00000 -40600	"
      " // electrodes Location ",
  "Filtering matrix ElectrodeCondition= "
      "{ 1 2 3 4 5 } " // row labels
      "{ Name Reference DispMin DispMax State Operator Value State Operator Value } " // column labels
      "realtime -1 0 10 TargetCode > 0 Feedback = 1 "
      "up 1 0 1 TargetCode = 1 Feedback = 1 "
      "right 1 0 1 TargetCode = 2 Feedback = 1 "
      "down 1 0 1 TargetCode = 3 Feedback = 1 "
      "left 1 0 1 TargetCode = 4 Feedback = 1 "
      " // electrodes Condition ",
  "Filtering float LearningRateHist= 0.99 1.0 0.0 512.0 "
      "// learningrate for realtime display histogram baseline",
  "Filtering float LearningRateAverage= 0.999 1.0 0.0 512.0 "
      "// learningrate for realtime display average baseline",
  "Filtering int CircleRadius= 50 1000 1 512 "
      "// maximal score to be displayed in average display",
  "Filtering int StatisticDisplayType=           0 0 0 2"
      "// statistic type of display: "
          " 0: z-score,"
          " 1: difference-of-means,"
          " 2: r-square,"
          "(enumeration)",
  "Filtering int ScoreType=           0 0 0 1"
      "// type of score: "
          " 0: mahalanobis-distance,"
          " 1: neg-log-probability,"
          "(enumeration)",
  "Filtering int FeedbackType=           0 0 0 1"
      "// type of score: "
          " 0: log(score),"
          " 1: real-time display,"
          "(enumeration)",
  "Filtering floatlist AutoScaleChannelList= 0 % % "
     " // channels used to autoscale display or leave emtpy for all channels",
  "Filtering float LearningRateAutoScale= 0.99 1.0 0.0 512.0 "
      "// learningrate for realtime display histogram baseline",
 END_PARAMETER_DEFINITIONS


  bprocessing = false;
  bclicked    = false;

  int    retVal;

  // load sigfried.dll
  hinstLib = LoadLibrary(TEXT("sigfried.dll"));

  // check if sigfried.dll was successfully loaded
  if (hinstLib != NULL) {
    // load functions from sigfried.dll
    set_std_out                     = (SET_STD_OUT)                       GetProcAddress(hinstLib, TEXT("set_std_out"));
    set_malloc                      = (SET_MALLOC)                        GetProcAddress(hinstLib, TEXT("set_malloc"));
    set_free                        = (SET_FREE)                          GetProcAddress(hinstLib, TEXT("set_free"));
    load_model                      = (LOAD_MODEL)                        GetProcAddress(hinstLib, TEXT("load_model"));
    get_feature_size_sample         = (GET_FEATURE_SIZE_SAMPLE)           GetProcAddress(hinstLib, TEXT("get_feature_size_sample"));
    build_data_sample_init          = (BUILD_DATA_FREQUENCY_SAMPLE_INIT)  GetProcAddress(hinstLib, TEXT("build_data_frequency_sample_init"));
    build_data_sample               = (BUILD_DATA_FREQUENCY_SAMPLE)       GetProcAddress(hinstLib, TEXT("build_data_frequency_sample"));
    build_data                      = (BUILD_DATA_FREQUENCY)              GetProcAddress(hinstLib, TEXT("build_data_frequency"));
    normalize_data                  = (NORMALIZE_DATA)                    GetProcAddress(hinstLib, TEXT("normalize_data"));
    score_data                      = (SCORE_DATA)                        GetProcAddress(hinstLib, TEXT("score_data"));

    // set the std to the printf function for debugging reasons
    retVal = (set_std_out)(&printf);

    // check if last call was successfull
    if (retVal != SIGFRIED_OK) {
      bcierr << "Unsuccesfull call of set_std_out." << endl;
    }

    // set the memory allocation to the calloc from this process
    retVal = (set_malloc) (&calloc);

    // check if last call was successfull
    if (retVal != SIGFRIED_OK) {
      bcierr << "Unsuccesfull call of set_malloc." << endl;
    }

    // set the memory deallocation to the free from this process
    retVal = (set_free)   (&free);

    // check if last call was successfull
    if (retVal != SIGFRIED_OK) {
      bcierr << "Unsuccesfull call of set_free." << endl;
    }

  }

  binitialized       = false;

}


// **************************************************************************
// Function:   ~SigfriedARFilter
// Purpose:    This is the destructor for the SigfriedARFilter class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SigfriedARFilter::~SigfriedARFilter()
{

  // deallocate the allocated instances
  if (binitialized) {
    for (unsigned int index_model=0; index_model < num_models; index_model++) {
      for (unsigned int index_condition=0; index_condition < velectrodecollections[index_model].size(); index_condition++) {
        delete velectrodecollections[index_model][index_condition];
        velectrodecollections[index_model][index_condition] = NULL;
      }
    }
  }
  binitialized = false;

  for (unsigned int index_model=0; index_model < num_models; index_model++) {

   delete vpForm[index_model];
   delete vpElectrodeRenderer[index_model];

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
void SigfriedARFilter::Preflight( const SignalProperties& inSignalProperties,
                                    SignalProperties& outSignalProperties ) const
{

  unsigned int                      num_models;

  const char                       *sz_filename;

  int                               errorcode;
  CVector<char>                     szerrorcode;
  CVector<SIGFRIED_MODEL_TYPE>      vmodel_old;

  CVector<float>                    vtrain_min_old;
  CVector<float>                    vtrain_max_old;

  DATA2MODEL_PARAMETERS_CMD         parameters_cmd;
  SIGFRIED_PARAMETERS_CFG           parameters_cfg;
  SIGFRIEDmodeller_PARAMETERS       parameters_sig;


  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  Parameter( "SampleBlockSize" );
  Parameter( "SamplingRate" );


  // Input signal checks.

// hack pb:
//  for( size_t channel = 0; channel < inSignalProperties.Channels(); ++channel )
//    PreflightCondition( inSignalProperties.GetNumElements( channel ) > 0 );

  // ElectrodeLocation
  int num_electrodelocation_rows = Parameter("ElectrodeLocation")->NumRows();
  int num_electrodelocation_columns = Parameter("ElectrodeLocation")->NumColumns();

  PreflightCondition(num_electrodelocation_rows  > 0);
  PreflightCondition(num_electrodelocation_columns  == 3);

  // ElectrodeCondition
  int num_electrodecondition_rows = Parameter("ElectrodeCondition")->NumRows();
  int num_electrodecondition_columns  = Parameter("ElectrodeCondition")->NumColumns();

  // check if the number of columns is a multiple after the first 4 columns is a multiple of 3
  PreflightCondition((num_electrodecondition_columns-4) % 3 == 0);

  // check if at least one condition exists
  PreflightCondition( num_electrodecondition_rows            > 0);

  // for each condition
  for (  int index_condition = 0;         index_condition         < num_electrodecondition_rows;     index_condition++) {

    int   reference = atoi(Parameter("ElectrodeCondition")->Value(index_condition,1).c_str());
    float dispMin   = atof(Parameter("ElectrodeCondition")->Value(index_condition,2).c_str());
    float dispMax   = atof(Parameter("ElectrodeCondition")->Value(index_condition,3).c_str());

    // check if reference is valid
    PreflightCondition(reference == -1 || (reference > 0 && reference <= num_electrodecondition_rows));

    // check if maximal display range is higher than the minimum display range
    PreflightCondition(dispMax > dispMin);

    // for each element of this condition
    for (int index_condition_element = 0; index_condition_element < (num_electrodecondition_columns-4) / 3; index_condition_element++) {

      // test if the defined state exists
      State(Parameter("ElectrodeCondition")->Value(index_condition,4+index_condition_element*3 + 0));

      // test if the defined operator is valid
      string szoperator = Parameter("ElectrodeCondition")->Value(index_condition,4+index_condition_element*3 + 1);
      PreflightCondition(szoperator == "=" || szoperator == ">" || szoperator == "<");
    }
  }


  // get number of models
  num_models = Parameter("ModelFiles")->NumRows();

  if (Parameter("ModelFiles")->NumColumns() != 2 && Parameter("ModelFiles")->NumColumns() != 5) {
    bcierr << "The number of columns in parameter ModelFiles has to be either 2 (filename, label) or 5 (filename, label, x-pos, y-pos, width)." << endl;
  }

  // check each model
  for (unsigned int index_model = 0; index_model < num_models; index_model++) {

    sz_filename = Parameter("ModelFiles")->Value(index_model,0).c_str();

    if (hinstLib != NULL) {

      //  try to load the model
      errorcode = load_model((char *)sz_filename,
                            &vmodel_old,
                            &vtrain_min_old,
                            &vtrain_max_old,
                            &parameters_cfg,
                            &parameters_cmd,
                            &parameters_sig,
                            &szerrorcode,
                            NULL);

      // check if a valid model was loaded
      PreflightCondition(errorcode == SIGFRIED_OK);

      // check if a valid model was loaded and display error message
      if (errorcode != SIGFRIED_OK) {
        bcierr << szerrorcode.getPtr() << endl;
      }

    } else {
      bcierr << "Sigfried DLL is not found." << endl;
    }

    // crosscheck model with parameters
    if ( Parameter( "TransmitChList" )->NumValues() != parameters_cfg.vchannels.getDimN() ) {
      bcierr << "Number of channels in TransmitChList (" << Parameter( "TransmitChList" )->NumValues() << ") != " <<
                "number of channels in Model ("          << parameters_cfg.vchannels.getDimN() << ")." << endl;
    }

    if (num_electrodelocation_rows != parameters_cfg.vchannels.getDimN()) {
      bcierr << "Number of channels in ElectrodeLocation (" << num_electrodelocation_rows << ") != " <<
                "number of channels in Model ("            << parameters_cfg.vchannels.getDimN() << ")." << endl;
    }

    if (Parameter( "SamplingRate" ) != parameters_cfg.parms[6]) {
      bcierr << "SamplingRate in Parameteters (" << Parameter( "SamplingRate" )
        << ") != SamplingRate in Model (" << parameters_cfg.parms[6] << ")." << endl;
    }

    if (Parameter( "SampleBlockSize" ) * Parameter( "MemWindows" ) != parameters_cfg.spectral_size) {
      bciout << "SampleBlockSize*MemWindows in Parameters (" << Parameter( "SampleBlockSize" ) * Parameter( "MemWindows" )
             << ") != spectral_size (" << parameters_cfg.spectral_size << ") in Model." << endl;
    }



  }


  // get display type
  int StatisticDisplayType = Parameter( "StatisticDisplayType" );

  // check display type
  if (StatisticDisplayType < 0 || StatisticDisplayType > 2) {
   bcierr << "Unkown setting in StatisticDisplayType." << endl;
  }

  // get score type
  int ScoreType = Parameter( "ScoreType" );

  // check score type
  if (ScoreType != mahalanobis_distance && ScoreType != neg_log_probability) {
    bcierr << "Unkown setting in ScoreType." << endl;
  }

  // get feedback type
  int feedback_type = Parameter( "FeedbackType" );

  // check feedback type
  if (feedback_type != log_score && feedback_type != real_time_display) {
    bcierr << "Unkown setting in FeedbackType." << endl;
  }

  // warning if r-square display type is used
  if (StatisticDisplayType == 2) {
    bciout << "Using 'r-square' in StatisticDisplayType is not recomended unless the time period for both conditions that are referenced are equal." << endl;;
  }

  // check SourceChGain and SourceChOffset to have the same number of entities
  if (Parameter("SourceChGain")->NumValues() != Parameter("SourceChOffset")->NumValues()) {
    bcierr << "Parameter SourceChGain must have the same number of entries as SourceChOffset." << endl;
  }

  // check the VisualizeSourceTime to have only positive values
  if (Parameter( "VisualizeSourceTime" ) <= 0) {
    bcierr << "Parameter VisualizeSourceTime must be > 0." << endl;
  }

  // check the AutoScaleChannelList to have not more values than the TransmitChList
  if (Parameter( "AutoScaleChannelList" )->NumValues() > Parameter( "TransmitChList" )->NumValues()) {
    bcierr << "Parameter AutoScaleChannelList has more entries than TransmitChList." << endl;
  }

  // check the AutoScaleChannelList to have not just one channel
  if (Parameter( "AutoScaleChannelList" )->NumValues() == 1) {
    bcierr << "Parameter AutoScaleChannelList must have either no or more then one entry." << endl;
  }

  // check each channel in the parameter AutoScaleChannelList
  if (Parameter( "AutoScaleChannelList" )->NumValues() > 0) {
    for (int idx_channel = 0; idx_channel < Parameter( "AutoScaleChannelList" )->NumValues(); idx_channel++) {

      // check for channels numbers higher than allowed
      if (Parameter( "AutoScaleChannelList" )(idx_channel) > Parameter( "TransmitChList" )->NumValues()) {
        bcierr << "Parameter AutoScaleChannelList item #" << idx_channel+1 << " (" << Parameter( "AutoScaleChannelList" )(idx_channel) << ")"
               << "higher than the highest channel number (" << Parameter( "TransmitChList" )->NumValues() << ")." << endl;
      }

      // check for channel numbers lower than allowed
      if (Parameter( "AutoScaleChannelList" )(idx_channel) < 1) {
        bcierr << "Parameter AutoScaleChannelList item #" << idx_channel+1 << " (" << Parameter( "AutoScaleChannelList" )(idx_channel) << ")"
               << "lower than the lowest channel number (1)." << endl;

      }
    }
  }

  outSignalProperties = SignalProperties( parameters_cfg.vchannels.getMax(), num_models );
  outSignalProperties.SetName( "SIGFRIED feedback" );

}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the SigfriedARFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void SigfriedARFilter::Initialize( const SignalProperties&, const SignalProperties& )
{


  char                             cur_buf[512];
  int                              width_condition;
  TRect                            boundary;
  CVector<char>                    szerrorcode;
  int                              errorcode;
  int                              numSamplesInOutputDisplay;

  // Destroy any existing display.
  DestroyDisplay();

  // get the geometric properties of the screen
  ScreenWidth         = GetSystemMetrics( SM_CXSCREEN );
  ScreenHeight        = GetSystemMetrics( SM_CYSCREEN );
  ScreenLeft          = GetSystemMetrics( SM_XVIRTUALSCREEN );
  ScreenTop           = GetSystemMetrics( SM_YVIRTUALSCREEN );

  // get the parameters
  SampleBlockSize         = Parameter( "SampleBlockSize" );
  MemWindows              = Parameter( "MemWindows" ) ;
  VisualizeSigfried       = Parameter( "VisualizeSigfried" );
  SamplingRate            = Parameter( "SamplingRate" );
  LearningRateHist        = Parameter( "LearningRateHist" );
  LearningRateAverage     = Parameter( "LearningRateAverage" );
  LearningRateAutoScale   = Parameter( "LearningRateAutoScale" );
  CircleRadius            = Parameter( "CircleRadius" );
  StatisticDisplayType    = Parameter( "StatisticDisplayType" );
  ScoreType               = Parameter( "ScoreType" );

  // get the number of models and thus number identities for all vectors
  num_models = Parameter("ModelFiles")->NumRows();

  // resize all vectors to accomodate this number of models
  vmodel_filename.resize      (num_models);
  vmodel_label.resize         (num_models);
  vmodel.resize               (num_models);
  vparameters_cfg.resize      (num_models);
  vparameters_cmd.resize      (num_models);
  vparameters_sig.resize      (num_models);
  vtrain_min.resize           (num_models);
  vtrain_max.resize           (num_models);
  vfeaturesamples.resize      (num_models);
  vfeaturesamples_out.resize  (num_models);
  vscore.resize               (num_models);
  vscore_display.resize       (num_models);
  vform_position.resize       (num_models);
  velectrodecollections.resize(num_models);
  vpCheckBox.resize           (num_models);
  vbRender.resize             (num_models);

  // for each model load the model and initialize the variables
  for (unsigned int index_model = 0; index_model < num_models; index_model++) {

    // get the model filename and label
    vmodel_filename[index_model]       = Parameter("ModelFiles")->Value(index_model,0).c_str();
    vmodel_label[index_model]          = Parameter("ModelFiles")->Value(index_model,1).c_str();

    // get the screen coordinates for the display associated with this model
    if (Parameter("ModelFiles")->NumColumns() == 5) {
      vform_position[index_model].Left   = atoi(Parameter("ModelFiles")->Value(index_model,2).c_str());
      vform_position[index_model].Top    = atoi(Parameter("ModelFiles")->Value(index_model,3).c_str());
      vform_position[index_model].Width  = atoi(Parameter("ModelFiles")->Value(index_model,4).c_str());
    } else {
      // if coordinates are provided use the default parameters which fill the screen
      vform_position[index_model].Left   = 0;
      vform_position[index_model].Top    = 0;
      vform_position[index_model].Width  = ScreenWidth;
    }

    // load model
    if (hinstLib != NULL) {
      errorcode = (load_model)((char *)vmodel_filename[index_model],
                  &vmodel[index_model],
                  &vtrain_min[index_model],
                  &vtrain_max[index_model],
                  &vparameters_cfg[index_model],
                  &vparameters_cmd[index_model],
                  &vparameters_sig[index_model],
                  &szerrorcode,
                  NULL);

      if (errorcode != SIGFRIED_OK) {
        bcierr << szerrorcode.getPtr() << endl;
      }

    } // if (hinstLib != NULL)

    // get the number of channels in the model
    num_channels = vparameters_cfg[index_model].vchannels.getDimN();

    // allocate temporary memory for the parameter of this model 
    CVector<int>   vfeature_size(feature_size,3);
    CVector<float> vparms(vparameters_cfg[index_model].parms,7);

    // initalize the build_data_sample_init which will extract the features
    // in the #Process function.
    (build_data_sample_init)(index_model,&vparameters_cfg[index_model]);

    // get the size of the expectet feature vector.
    (get_feature_size_sample)(index_model,vfeature_size);

    // allocate the memory for the expected feature vector and score
    vfeaturesamples[index_model].resize        (feature_size[0],feature_size[1],feature_size[2],calloc,free);
    vfeaturesamples_out[index_model].resize    (feature_size[0],feature_size[1],feature_size[2],calloc,free);
    vscore[index_model].resize                 (1,num_channels,calloc,free);
    vscore_display[index_model].resize         (1,num_channels,calloc,free);

  } // for (unsigned int index_model = 0; index_model < num_models; index_model++)


  // Initialize the autoscale feature by extracting the channels that will
  // be used to estimate the autoscale range.
  if (Parameter( "AutoScaleChannelList" )->NumValues() > 0) {

    this->vAutoScaleChannelList.resize(Parameter( "AutoScaleChannelList" )->NumValues());

    for (int idx_channel = 0; idx_channel < Parameter( "AutoScaleChannelList" )->NumValues(); idx_channel++) {
      this->vAutoScaleChannelList.at(idx_channel) = Parameter( "AutoScaleChannelList" )(idx_channel);
    }
  } else {
    // If no channels were provided use all channels. 
    for (int idx_channel = 0; idx_channel < Parameter( "AutoScaleChannelList" )->NumValues(); idx_channel++) {
      this->vAutoScaleChannelList.at(idx_channel) = idx_channel;
    }
  } // if (Parameter( "AutoScaleChannelList" )->NumValues() > 0)

  // Create new displays that fit the new parameters.
  CreateDisplay();

  // Get the ElectrodeLocation description.
  num_electrodelocation_rows     = Parameter("ElectrodeLocation")->NumRows();
  num_electrodelocation_columns  = Parameter("ElectrodeLocation")->NumColumns();

  electrodesdescription.resize(num_electrodelocation_rows);

  // Initialize the variables that will contain min/max coordinates.
  electrodeposxmin =  1e32;
  electrodeposxmax = -1e32;
  electrodeposymin =  1e32;
  electrodeposymax = -1e32;

  // Iterate through the electrodes
  for (int index_electrode = 0; index_electrode < num_electrodelocation_rows; index_electrode++) {

    // Store label and position of this electrode
    electrodesdescription[index_electrode].szlabel = Parameter("ElectrodeLocation")->Value(index_electrode,0);
    electrodesdescription[index_electrode].posx    = atof(Parameter("ElectrodeLocation")->Value(index_electrode,1).c_str());
    electrodesdescription[index_electrode].posy    = atof(Parameter("ElectrodeLocation")->Value(index_electrode,2).c_str());

    // Find min/max coordinates
    if (electrodesdescription[index_electrode].posx < electrodeposxmin) {
      electrodeposxmin=electrodesdescription[index_electrode].posx;
    }
    if (electrodesdescription[index_electrode].posx > electrodeposxmax) {
      electrodeposxmax=electrodesdescription[index_electrode].posx;
    }
    if (electrodesdescription[index_electrode].posy < electrodeposymin) {
      electrodeposymin=electrodesdescription[index_electrode].posy;
    }
    if (electrodesdescription[index_electrode].posy > electrodeposymax) {
      electrodeposymax=electrodesdescription[index_electrode].posy;
    }

  }

  // calculate aspect ratio
  if (electrodeposxmax - electrodeposxmin > 0 && electrodeposymax - electrodeposymin > 0) {
    electrodeaspectratio = (electrodeposymax - electrodeposymin) / (electrodeposxmax - electrodeposxmin);
  } else {
    // if singular aspect ratio use aspect ratio 1
    electrodeaspectratio = 1;
  }

  min_elect_dist = 1e16;
  // calculate electrode distance
  for (int index_electrodeA = 0; index_electrodeA < num_electrodelocation_rows-1; index_electrodeA++) {
    for (int index_electrodeB = index_electrodeA+1; index_electrodeB < num_electrodelocation_rows; index_electrodeB++) {
      float distx  = electrodesdescription[index_electrodeA].posx - electrodesdescription[index_electrodeB].posx;
      float disty  = electrodesdescription[index_electrodeA].posy - electrodesdescription[index_electrodeB].posy;
      float distr  = sqrt(distx*distx+disty*disty);
      if (distr < min_elect_dist) min_elect_dist=distr;
    }
  }

  // free memory from last call
  if (binitialized) {

    for (unsigned int index_model=0; index_model < num_models; index_model++) {

      for (unsigned int index_condition=0; index_condition < velectrodecollections[index_model].size(); index_condition++) {
        delete velectrodecollections[index_model][index_condition];
        velectrodecollections[index_model][index_condition] = NULL;
        vpCheckBox[index_model][index_condition]->Visible = false;
        delete vpCheckBox[index_model][index_condition];
      }

      delete vpElectrodeRenderer[index_model];
      vpElectrodeRenderer[index_model] = new CElectrodeRenderer(vpForm[index_model]);

    }
    binitialized = false;
  }

  // Get the ElectrodeCondition description.
  num_electrodecondition_rows     = Parameter("ElectrodeCondition")->NumRows();
  num_electrodecondition_columns  = Parameter("ElectrodeCondition")->NumColumns();

  // Allocate memory for all conditions.
  conditions.resize(num_electrodecondition_rows);

  // Initialize the conditions.
  for (  int index_condition = 0;         index_condition         < num_electrodecondition_rows;     index_condition++) {
    conditions[index_condition].reference = -1;
  }

  // Decode the condition elements from the parameter.
  for (  int index_condition = 0;         index_condition         < num_electrodecondition_rows;     index_condition++) {

    // Allocate memory for the elements of this condition.
    conditions[index_condition].conditionelements.resize((num_electrodecondition_columns-4) / 3);

    // Store the name of this condition.
    conditions[index_condition].szname    = Parameter("ElectrodeCondition")->Value(index_condition,0);

    // The reference condition for this condition.
    int reference   = atoi(Parameter("ElectrodeCondition")->Value(index_condition,1).c_str());

    // The min/max radius of the circles.
    float dispMin   = atof(Parameter("ElectrodeCondition")->Value(index_condition,2).c_str());
    float dispMax   = atof(Parameter("ElectrodeCondition")->Value(index_condition,3).c_str());
    conditions[index_condition].dispMin = dispMin;
    conditions[index_condition].dispMax = dispMax;

    // Store at the the referenced condition that this is it's reference.
    if (reference != -1) {
      conditions[index_condition].reference = reference;
    }

    // Iterate through all elements of this condition and store state, operator and value for this logically with and && operator connected elements.
    for (int index_condition_element = 0; index_condition_element < (num_electrodecondition_columns-4) / 3; index_condition_element++) {
      conditions[index_condition].conditionelements[index_condition_element].szstate    =      Parameter("ElectrodeCondition")->Value(index_condition,4+index_condition_element*3 + 0);
      conditions[index_condition].conditionelements[index_condition_element].szoperator =      Parameter("ElectrodeCondition")->Value(index_condition,4+index_condition_element*3 + 1);
      conditions[index_condition].conditionelements[index_condition_element].value      = atoi(Parameter("ElectrodeCondition")->Value(index_condition,4+index_condition_element*3 + 2).c_str());
    }
  }

  // For each model.
  for (unsigned int index_model = 0; index_model < num_models; index_model++) {

    // Allocate memory for the collections of this model.
    velectrodecollections[index_model].resize(num_electrodecondition_rows);

    // Get the width of this from.
    width_condition = (vpForm[index_model]->Width / num_electrodecondition_rows)-20;

    // Adjust the Height to the aspect ratio of the electrode collection.
    vpForm[index_model]->Height = width_condition*electrodeaspectratio + 140 + 18 * num_electrodecondition_rows;

    // For each condition and thus each collection display.
    for (int index_condition=0; index_condition < num_electrodecondition_rows; index_condition++) {
      if (index_condition == 0) {
        // Create a real-time display
        velectrodecollections[index_model][index_condition]   = new CElectrodeCollection(conditions[index_condition].szname,string(""),string(""),0,0,LearningRateHist,LearningRateAverage,0.1,false,false,StatisticDisplayType,vAutoScaleChannelList,LearningRateAutoScale);
      } else {
        // Create a referenced display.
        if (conditions[index_condition].reference > 0) {
          velectrodecollections[index_model][index_condition] = new CElectrodeCollection(conditions[index_condition].szname,"","",0,0,0,0,0,true,true,StatisticDisplayType,vAutoScaleChannelList,LearningRateAutoScale);
        } else {
          // Create a averaged display.
          velectrodecollections[index_model][index_condition] = new CElectrodeCollection(conditions[index_condition].szname,"","",0,0,0,0,0,true,false,StatisticDisplayType,vAutoScaleChannelList,LearningRateAutoScale);
        }

      }

      // For each electrode.
      for (int index_electrode = 0; index_electrode < num_electrodelocation_rows; index_electrode++) {

        // Create an identity for this electrode in the electrode collection.
        velectrodecollections[index_model][index_condition]->AddElectrodeCircle(electrodesdescription[index_electrode].posx,
                                                                                electrodesdescription[index_electrode].posy,
                                                                                0,
                                                                                electrodesdescription[index_electrode].szlabel,
                                                                                clRed);

      }

      // Get the boundary box for this collection.
      boundary = velectrodecollections[index_model][index_condition]->GetBoundingBoxElectrodes();

      // Add first two collections (realtime and average)
      if (index_condition == 0 || index_condition == 1) {


        vpElectrodeRenderer[index_model]->AddCollection(velectrodecollections[index_model][index_condition],
                                                        TRect(boundary.Left-1000,boundary.top-1000,boundary.right+1000,boundary.bottom+1000),
                                                        TRect(index_condition*width_condition+10,0,(index_condition+1)*width_condition,width_condition*electrodeaspectratio),
                                                        conditions[index_condition].dispMin,
                                                        conditions[index_condition].dispMax,
                                                        1,
                                                        CircleRadius);

      } else {

        // Add furhter collections (referenced)
        vpElectrodeRenderer[index_model]->AddCollection(velectrodecollections[index_model][index_condition],
                                                        TRect(boundary.Left-1000,boundary.top-1000,boundary.right+1000,boundary.bottom+1000),
                                                        TRect(index_condition*width_condition+10,0,(index_condition+1)*width_condition,width_condition*electrodeaspectratio),
                                                        conditions[index_condition].dispMin,
                                                        conditions[index_condition].dispMax,
                                                        1,
                                                        CircleRadius);

      }
    }

    // Allcoate memory for the checkbox that turns collections on/off
    vpCheckBox[index_model].resize(num_electrodecondition_rows);

    // Allocate memory for the marker that indicates that a collection is being rendered.
    vbRender[index_model].resize(num_electrodecondition_rows);

    // For each condtion.
    for (  int index_condition = 0;         index_condition         < num_electrodecondition_rows;     index_condition++) {

      // Create a new check box identity.
      vpCheckBox[index_model][index_condition]                        = new TCheckBox(static_cast<TComponent*>(NULL));
      vpCheckBox[index_model][index_condition]->Parent                = vpForm[index_model];
      vpCheckBox[index_model][index_condition]->Top                   = 0;
      vpCheckBox[index_model][index_condition]->Left                  = 0;
      vpCheckBox[index_model][index_condition]->Font->Size            = 10;
      vpCheckBox[index_model][index_condition]->Font->Style           = TFontStyles() << fsBold;
      vpCheckBox[index_model][index_condition]->Alignment             = taCenter;
      vpCheckBox[index_model][index_condition]->Visible               = true;
      vpCheckBox[index_model][index_condition]->Caption               = conditions[index_condition].szname.c_str();
      vpCheckBox[index_model][index_condition]->Checked               = true;
      vpCheckBox[index_model][index_condition]->OnClick               = ClickedBox;

      // And initialize it as checked.
      vbRender[index_model][index_condition] = vpCheckBox[index_model][index_condition]->Checked;


    }

  }

  // Mark that everthing is initialized and resize the form.
  binitialized = true;
  FormResize(NULL);

  // Finally show the forms.
  for (unsigned int index_model = 0; index_model < num_models; index_model++) {
    vpForm[index_model]->Visible = true;
  }


  // If the SIGFRIED results are to be visualized in a seperate graph.
  if ( VisualizeSigfried == 1 ) {

    // Mark that the visualization is enabled.
    visualize=true;

    // Get the number of samples that should be displayed in the display.
    numSamplesInOutputDisplay = ( Parameter( "VisualizeSourceTime" ) * Parameter( "SamplingRate" ) / Parameter( "SampleBlockSize" ) );

    // Delete any old display.
    for (unsigned int index_model = 0; index_model < vscoreVis.size(); index_model++) {
      delete vscoreVis[index_model];
    }

    // Allocate memory for new visualized score graphs.
    vscoreVis.resize(num_models);

    for (unsigned int index_model = 0; index_model < num_models; index_model++) {

      AnsiString szdetails;

      // Title of the graph.
      szdetails.sprintf("SIGFRIED Score Graph (%s)",vmodel_label[index_model]);

      vscoreVis[index_model] = new GenericVisualization( SourceID::EMALG + index_model);
      vscoreVis[index_model]->Send(CfgID::WindowTitle, szdetails.c_str());
      vscoreVis[index_model]->Send(CfgID::NumSamples , numSamplesInOutputDisplay);
      vscoreVis[index_model]->Send(CfgID::MinValue   , conditions[0].dispMin);
      vscoreVis[index_model]->Send(CfgID::MaxValue   , conditions[0].dispMax);
    }

  } else {
    visualize=false;
  }



  // determine the SourceChGain for each channel
  vSourceChGain.resize(Parameter("SourceChGain")->NumValues());

  for (unsigned int ch=0; ch<vSourceChGain.size(); ch++)
    this->vSourceChGain.at(ch)=1.0 / Parameter("SourceChGain")(ch);

  // determine the SourceChOffset for each channel
  vSourceChOffset.resize(Parameter("SourceChOffset")->NumValues());

  for (unsigned int ch=0; ch<vSourceChOffset.size(); ch++)
    this->vSourceChOffset.at(ch)=Parameter("SourceChOffset")(ch);


  totaltimesum        = 0;
  counter             = 0;

// Multi-Threading: Not yet finished!
/*
  for (unsigned int index_model=0; index_model<num_models; index_model++) {

    workerthread[index_model].identity  = index_model;
    workerthread[index_model].pOwner    = this;
    workerthread[index_model].counter   = 0;


    thread[index_model] = CreateThread(0,0,TestThread,&workerthread[index_model], CREATE_SUSPENDED,NULL) ;


    eventworkerstart[index_model] = CreateEvent(
              NULL,   // default security attributes
              FALSE,  // auto-reset event object
              FALSE,  // initial state is nonsignaled
              NULL);  // unnamed object

    eventworkerfinished[index_model] = CreateEvent(
              NULL,   // default security attributes
              FALSE,  // auto-reset event object
              FALSE,  // initial state is nonsignaled
              NULL);  // unnamed object

  }
*/

}


// **************************************************************************
// Function:   Process
// Purpose:    This function applies the peak detector
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void SigfriedARFilter::Process(const GenericSignal& input, GenericSignal& output)
{
  LARGE_INTEGER   prectime, prectimebase;
  double          starttime, endtime, totaltime;
  char            memotext[256], str_builddata[256], str_calcscore[256];
  int             ocount, ncount;
  CVector<char>   szerrorcode;

  static          count= 0;


  // calculate the current time
  QueryPerformanceCounter(&prectime);
  QueryPerformanceFrequency(&prectimebase);
  starttime=(double)prectime.QuadPart/(double)prectimebase.QuadPart*1000;

  count++;

  // calculate window length
  winlgth = MemWindows * SampleBlockSize;

  // copy data from current sampleblock into the window
  for (int i=0; i<(int)input.Channels(); i++) {
    for(int j=MemWindows-1; j>0; j--) {
      for(int k=0; k<SampleBlockSize; k++) {
        ncount= j*SampleBlockSize + k;
        ocount= (j-1)*SampleBlockSize + k;
        datwin[i][ncount]= datwin[i][ocount];
      } // for(int k=0; k<SampleBlockSize; k++)
    } // for(int j=MemWindows-1; j>0; j--)

    count= SampleBlockSize;

    for(int j=0; j<SampleBlockSize; j++) {
      //
      count--;
      // store data from current sampleblock into window and undo the offset and gain
      datwin[i][j] = (input.Value(i,count) * vSourceChGain.at(i)) + vSourceChOffset.at(i);
    } // for(int j=0; j<SampleBlockSize; j++)

  } // for (int i=0; i<(int)input.Channels(); i++)


  vsignal.resize(MemWindows * SampleBlockSize, (int)input.Channels());


  float *sigptr = &vsignal(0,0);

  // copy data into sigfried data structure
  for (int samp=0; samp<winlgth; samp++)
    for (int ch=0; ch<(int)input.Channels(); ch++)
      sigptr[ch*winlgth+samp]=datwin[ch][samp];


// Multi-Threading: Not yet finished!
/*
   for (unsigned int index_model=0; index_model<num_models; index_model++) {
     SetEvent(eventworkerstart[index_model]);
   }

   for (unsigned int index_model=0; index_model<num_models; index_model++) {
     WaitForMultipleObjects(
          1,           // number of objects in array
          &eventworkerfinished[index_model],     // array of objects
          FALSE,       // wait for any object
          INFINITE);       // five-second wait
   }


   for (unsigned int index_model=0; index_model<num_models; index_model++) {
     ResetEvent(eventworkerfinished[index_model]);
   }
*/

  // For all models.
  for (unsigned int index_model=0; index_model < num_models; index_model++) {

    //
    // do SIGFRIED calculations starting from when the first data window is full
    //
    if (counter >= MemWindows) {

      // build maximum entropy features
      (build_data_sample)(index_model,&vsignal,&vfeaturesamples[index_model]);

      // normalize maximum entropy features to the range defined by the train_min & train_max in the model
      (normalize_data)(&vfeaturesamples[index_model],&vtrain_min[index_model],&vtrain_max[index_model],&vfeaturesamples_out[index_model],&szerrorcode,NULL);

      // calculate the SIGFRIED score
      if (ScoreType == mahalanobis_distance) {

        // mahalanobis-distance
        (score_data)  (&vparameters_cfg[index_model].vchannels,
                       &vmodel[index_model],
                       &vfeaturesamples_out[index_model],
                       NULL,
                       &vscore[index_model],
                       NULL,
                       &szerrorcode,
                       NULL);

      } else if (ScoreType == neg_log_probability) {

        // neg-log-probability
        (score_data)  (&vparameters_cfg[index_model].vchannels,
                       &vmodel[index_model],
                       &vfeaturesamples_out[index_model],
                       NULL,
                       NULL,
                       &vscore[index_model],
                       &szerrorcode,
                       NULL);

      } else {

        bcierr << "Unsupported setting in ScoreType." << endl;

      }



      //
      // for each condition send score to corresponding display
      //
      for (unsigned int index_condition = 0; index_condition < conditions.size(); index_condition++) {

        // initialize the status marker that tells if the current condition is true
        bool status_condition = true;

        // iterate through all elements of this condition
        for (unsigned int index_condition_element = 0; index_condition_element < conditions[index_condition].conditionelements.size(); index_condition_element++) {

          // state name that defines this condition element
          string condition_state  = conditions[index_condition].conditionelements[index_condition_element].szstate.c_str();

          // current state value of the state name that defines this condition element
          int condition_state_value_current      = State(condition_state);

          // reference state value of the state name that defines this condition element
          int condition_state_value_reference    = conditions[index_condition].conditionelements[index_condition_element].value;

          // operator for this condition element
          string szoperator = conditions[index_condition].conditionelements[index_condition_element].szoperator;

          // perform logical operation
          if (szoperator == "=") {
            status_condition = status_condition && (condition_state_value_current == condition_state_value_reference);
          } else if (szoperator == ">") {
            status_condition = status_condition && (condition_state_value_current >  condition_state_value_reference);
          } else if (szoperator == "<") {
            status_condition = status_condition && (condition_state_value_current <  condition_state_value_reference);
          } else {
            bcierr << "Unsupported operator in ElectrodeCondition." << endl;
          }
        } // for (int index_condition_element = 0; index_condition_element < conditions[index_condition].conditionelements.size(); index_condition_element++)

        // if this condition is true highlight the corresponding display block
        if (status_condition) {
          vpElectrodeRenderer[index_model]->Highlight(index_condition);
        } else {
          vpElectrodeRenderer[index_model]->HighlightOff(index_condition);
        }

        // If this condition is true set the values of the corresponding electrode collections.
        if (status_condition) {

          // First for the this condition.
          for (int index_electrode = 0; index_electrode < num_electrodelocation_rows; index_electrode++) {

              if (ScoreType == mahalanobis_distance) {
                velectrodecollections[index_model][index_condition]->GetElectrodeCircle(index_electrode)->SetValue(LOG(1+vscore[index_model](0,index_electrode)));
              } else if (ScoreType == neg_log_probability) {
                velectrodecollections[index_model][index_condition]->GetElectrodeCircle(index_electrode)->SetValue(vscore[index_model](0,index_electrode));
              } else {
                bcierr << "Unsupported setting in ScoreType." << endl;
              } // if (ScoreType == mahalanobis_distance)

            // Second for all referenced conditions.
            for (  unsigned int index_condition_ref = 0; index_condition_ref < conditions.size(); index_condition_ref++) {
              if (conditions[index_condition_ref].reference ==  (int)(index_condition+1)) {

                if (ScoreType == mahalanobis_distance) {
                  velectrodecollections[index_model][index_condition_ref]->GetElectrodeCircle(index_electrode)->SetValueReference(LOG(1+vscore[index_model](0,index_electrode)));
                } else if (ScoreType == neg_log_probability) {
                  velectrodecollections[index_model][index_condition_ref]->GetElectrodeCircle(index_electrode)->SetValueReference(vscore[index_model](0,index_electrode));
                } else {
                  bcierr << "Unsupported setting in ScoreType." << endl;
                } // if (ScoreType == mahalanobis_distance)

              } // if (conditions[index_condition_ref].reference ==  (index_condition+1))
            } // for (  int index_condition_ref = 0; index_condition_ref < conditions.size(); index_condition_ref++)
          } // for (int index_electrode = 0; index_electrode < num_electrodelocation_rows; index_electrode++)
        } // if (status_condition)
      } // for (int index_condition = 0; index_condition < conditions.size(); index_condition++)


      //
      // render display
      //
      vpElectrodeRenderer[index_model]->Process();
      vpElectrodeRenderer[index_model]->Render();

      //
      // provide feedback
      //

      // retrieve the current display values
      for (int index_electrode = 0; index_electrode < num_electrodelocation_rows; index_electrode++)
        vscore_display[index_model](0,index_electrode) = velectrodecollections[index_model][0]->GetElectrodeCircle(index_electrode)->GetValue();

      // go through all channels
      for(int ch=0; ch<num_channels; ch++) {

        float cur_output;

        // feedback is the derived from the score
        if (feedback_type == log_score) {

          // feedback is the logarithmic mahalanobis distance score
          if (ScoreType == mahalanobis_distance) {
            cur_output=LOG(1+vscore[index_model](0,ch));
          // feedback is the neg log probablity
          } else if (ScoreType == neg_log_probability) {
            cur_output=vscore[index_model](0,ch);
          } else {
            bcierr << "Unkown setting in ScoreType." << endl;
          }

        // feedback is the derived from the real time display
        } else if (feedback_type == real_time_display) {
          cur_output=vscore_display[index_model](0,ch);
        } else {
          bcierr << "Unsupported setting in FeedbackType." << endl;
        }

        // finally set the output
        int ch_out = vparameters_cfg[index_model].vchannels(ch)-1;
        output.SetValue( ch_out, index_model, cur_output);
      }


      //
      // render visualziation graph
      //
      if ( visualize ) {

        for (unsigned int index_model=0; index_model < num_models; index_model++) {
          vscoreVis[index_model]->Send(output);
          vscoreVis[index_model]->Send(CfgID::MinValue   , velectrodecollections[index_model][0]->GetMinValueElectrodes());
          vscoreVis[index_model]->Send(CfgID::MaxValue   , velectrodecollections[index_model][0]->GetMaxValueElectrodes());
        }

      }
    }

  }


  // calculate the current time
  QueryPerformanceCounter(&prectime);
  QueryPerformanceFrequency(&prectimebase);
  endtime=(double)prectime.QuadPart/(double)prectimebase.QuadPart*1000;

  totaltime = endtime - starttime;

  totaltimesum+=totaltime;
  counter++;

  return;


}


void __fastcall SigfriedARFilter::FormResize(TObject *Sender)
{

  // If window is already processing don't perform action.
  if (!bclicked) {
    if (bprocessing) return;
    if (bresizing) return;
  }

  // Mark that window is now being processed.
  bresizing = true;

  // Initialize variables.
  int width_condition               = 0;
  int num_electrodecondition_render = 0;
  int index_condition_render        = 0;

  // Only perform resizing if the window was initialized.
  if (binitialized) {

    // For each model.
    for (unsigned int index_model=0; index_model < num_models; index_model++) {

      // Initialize variables.
      width_condition               = 0;
      num_electrodecondition_render = 0;
      index_condition_render        = 0;

      // Reset its initialized status.
      binitialized = false;

      // ????
      for (  int index_condition = 0;         index_condition         < num_electrodecondition_rows;     index_condition++) {
        if (vbRender[index_model][index_condition]) num_electrodecondition_render++;
      }

      if (num_electrodecondition_render == 0) num_electrodecondition_render = 1;

      // Adjust the new form widht and height.
      width_condition = (vpForm[index_model]->Width / num_electrodecondition_render)-20;
      vpForm[index_model]->Height   = width_condition*electrodeaspectratio + 140 + 18 * num_electrodecondition_rows;

      // Get the scale factor.
      float scale = (float)vpForm[index_model]->Width / 1440;

      // Initialize Variables.
      index_condition_render=0;

      // For all conditions.
      for (int index_condition=0; index_condition < num_electrodecondition_rows; index_condition++) {

        // Get the bounding box.
        TRect boundary = velectrodecollections[index_model][index_condition]->GetBoundingBoxElectrodes();

        if (index_condition == 0 || index_condition == 1) {

          // Update the realtime and average display
          vpElectrodeRenderer[index_model]->UpdateCollectionDevice(index_condition,
                                                     TRect(index_condition_render*width_condition+10,0,(index_condition_render+1)*width_condition,width_condition*electrodeaspectratio),
                                                     1,
                                                     (float)CircleRadius * scale,
                                                     vbRender[index_model][index_condition]);



        } else {

          // Update the referenced displays
          vpElectrodeRenderer[index_model]->UpdateCollectionDevice(index_condition,
                                                     TRect(index_condition_render*width_condition+10,0,(index_condition_render+1)*width_condition,width_condition*electrodeaspectratio),
                                                     1,
                                                     (float)CircleRadius * scale,
                                                     vbRender[index_model][index_condition]);


        }

        // ????
        if (vbRender[index_model][index_condition]) index_condition_render++;
      }

      // Initialize the chekcboxes.
      for (  int index_condition = 0;         index_condition         < num_electrodecondition_rows;     index_condition++) {

        vpCheckBox[index_model][index_condition]->Top                   = width_condition*electrodeaspectratio + 80 + 18 * (index_condition+1);
        vpCheckBox[index_model][index_condition]->Left                  = 10;
      }
    }

    // Mark that everything is now initialized again.
    binitialized = true;

    // Render the changes.
    for (unsigned int index_model=0; index_model < num_models; index_model++) {
      vpElectrodeRenderer[index_model]->RenderInit(10,20);
      vpElectrodeRenderer[index_model]->Process(true);
      vpElectrodeRenderer[index_model]->Render();
    }

  }

  bresizing = false;

}


void __fastcall SigfriedARFilter::FormShow(TObject *Sender)
{
  if (binitialized & !bprocessing) {
    for (unsigned int index_model=0; index_model < num_models; index_model++) {
      vpElectrodeRenderer[index_model]->Render();
    }
  }
}


void __fastcall SigfriedARFilter::ClickedBox(TObject *Sender)
{

  for (unsigned int index_model=0; index_model < num_models; index_model++) {
    for (int index_condition=0; index_condition < num_electrodecondition_rows; index_condition++) {
      vbRender[index_model][index_condition] = vpCheckBox[index_model][index_condition]->Checked;
    }
  }

  bclicked = true;

  FormResize(NULL);

  bclicked = false;

}


void SigfriedARFilter::StartRun()
{
  bprocessing             = true;

  TBorderIcons BorderIcons = TBorderIcons() >> biSystemMenu;

  for (unsigned int index_model=0; index_model < num_models; index_model++) {
    vpForm[index_model]->BorderIcons = BorderIcons;
    vpElectrodeRenderer[index_model]->RenderInit(10,20);
    vpElectrodeRenderer[index_model]->Render();
  }

// Multi-Threading: Not yet finished!
/*
  for (unsigned int index_model=0; index_model < num_models; index_model++) {
    ResumeThread(thread[index_model]);
  }
*/  

}

void SigfriedARFilter::StopRun()
{

  TBorderIcons BorderIcons = TBorderIcons() << biSystemMenu << biMinimize << biMaximize;

  for (unsigned int index_model=0; index_model < num_models; index_model++) {
    vpForm[index_model]->BorderIcons = BorderIcons;

    Parameter("ModelFiles")(index_model,2) = vpForm[index_model]->Left;
    Parameter("ModelFiles")(index_model,3) = vpForm[index_model]->Top;
    Parameter("ModelFiles")(index_model,4) = vpForm[index_model]->Width;

  }

  bprocessing             = false;

  FormResize(NULL);


}


void SigfriedARFilter::CreateDisplay()
{

  vpForm.resize(num_models);
  vpElectrodeRenderer.resize(num_models);

  for (unsigned int index_model = 0; index_model < num_models; index_model++) {


    AnsiString szdetails;
    szdetails.sprintf("SIGFRIED Score Topography (%s)",vmodel_label[index_model]);

    // create a form that will contain the SIGFRIED display
    vpForm[index_model]               = new TSigfriedForm;
    vpForm[index_model]->pOwner       = this;
    vpForm[index_model]->Visible      = false;
    vpForm[index_model]->Left         = vform_position[index_model].Left;
    vpForm[index_model]->Top          = vform_position[index_model].Top;
    vpForm[index_model]->Width        = vform_position[index_model].Width;
    vpForm[index_model]->Height       = vform_position[index_model].Width/2.5;
    vpForm[index_model]->Color        = clWhite;
    vpForm[index_model]->Caption      = szdetails;
    vpForm[index_model]->DisableAutoRange();


    bresizing = false;

    // connect the function pointers to the implemented functions
    vpForm[index_model]->OnResize     = FormResize;
    vpForm[index_model]->OnShow       = FormShow;
    vpForm[index_model]->OnPaint      = FormShow;

    // note that display was not initalized yet
    binitialized        = false;

    // create a instance of the electrode renderer
    vpElectrodeRenderer[index_model]  = new CElectrodeRenderer(vpForm[index_model]);
    
    vpForm[index_model]->OnMouseMove  = vpElectrodeRenderer[index_model]->OnMouseMove;
  }

  FormResize(NULL);


}


void SigfriedARFilter::DestroyDisplay()
{

  // deallocate the allocated instances
  if (binitialized) {

    for (unsigned int index_model=0; index_model < vpForm.size(); index_model++) {
      for (unsigned int index_condition=0; index_condition < velectrodecollections[index_model].size(); index_condition++) {
        delete velectrodecollections[index_model][index_condition];
        velectrodecollections[index_model][index_condition] = NULL;
      }
    }
  }

  binitialized = false;

  for (unsigned int index_model=0; index_model < vpForm.size(); index_model++) {

    if (vpForm[index_model] != NULL) {

      vform_position[index_model].Left  = vpForm[index_model]->Left;
      vform_position[index_model].Top   = vpForm[index_model]->Top;
      vform_position[index_model].Width = vpForm[index_model]->Width;

      delete vpForm[index_model];
    }

    if (vpElectrodeRenderer[index_model] !=  NULL) {
      delete vpElectrodeRenderer[index_model];
    }

    vpForm[index_model]              = NULL;
    vpElectrodeRenderer[index_model] = NULL;

  }

}

// Multi-Threading: Not yet finished!
/*
DWORD WINAPI SigfriedARFilter::TestThread(LPVOID param)
{


  while(1) {

    WaitForMultipleObjects(
        1,           // number of objects in array
        &((TWORKERTHREAD*)param)->pOwner->eventworkerstart[((TWORKERTHREAD*)param)->identity],     // array of objects
        FALSE,       // wait for any object
        INFINITE);       // five-second wait

    ResetEvent(((TWORKERTHREAD*)param)->pOwner->eventworkerstart[((TWORKERTHREAD*)param)->identity]);

//    Sleep(20);

    (((TWORKERTHREAD*)param)->pOwner)->ProcessWorkerThread(((TWORKERTHREAD*)param)->identity);

//    bciout << ((TWORKERTHREAD*)param)->identity << ":" << ((TWORKERTHREAD*)param)->counter++ << endl;

    SetEvent(((TWORKERTHREAD*)param)->pOwner->eventworkerfinished[((TWORKERTHREAD*)param)->identity]);

  }

  return 0;


}
*/


// Multi-Threading: Not yet finished!
/*
void SigfriedARFilter::ProcessWorkerThread(int index_model)
{

}
*/
