#ifndef DATATYPES_H_INCLUDED
#define DATATYPES_H_INCLUDED                                                   

enum defaults {
  SKIPROWS      = 3,
  SKIPCOLUMNS   = 1
};


enum errorcodes {
  SIGFRIED_OK = 0,
  SIGFRIED_EXCEPTION,
  SIGFRIED_DATA2MODEL_DLL_LOAD_FAILED,
  SIGFRIED_DATA2MODEL_DLL_NOT_EXPORTS_ALL_FUNCTIONS,
  SIGFRIED_DATA2MODEL_CMDLINE_INVALID,
  SIGFRIED_DATA2MODEL_INIFILE_NOT_EXIST,
  SIGFRIED_DATA2MODEL_INIFILE_INVALID,
  SIGFRIED_DATA2MODEL_INIFILE_INVALID_START,
  SIGFRIED_DATA2MODEL_INIFILE_INVALID_STOP,
  SIGFRIED_DATA2MODEL_INIFILE_INVALID_DELTA,
  SIGFRIED_DATA2MODEL_INIFILE_INVALID_BANDWIDTH,
  SIGFRIED_DATA2MODEL_INIFILE_INVALID_DETREND,
  SIGFRIED_DATA2MODEL_INIFILE_INVALID_MODELORDER,
  SIGFRIED_DATA2MODEL_INIFILE_INVALID_SAMPLEFREQ,
  SIGFRIED_DATA2MODEL_SIGNAL_FILENOTEXIST,
  SIGFRIED_DATA2MODEL_SIGNAL_ERRORREAD,
  SIGFRIED_DATA2MODEL_PROCESS_BUFFER_TO_SMALL,
  SIGFRIED_DATA2SCORE_SCORE_FILE_ERROR,
  SIGFRIED_NUM_SAMPLES_NEGATIVE,
  SIGFRIED_PARAMS_SPECTRAL_SIZE,
  SIGFRIED_PARAMS_SPECTRAL_STEPPING,
  SIGFRIED_PARAMS_START,
  SIGFRIED_PARAMS_STOP,
  SIGFRIED_PARAMS_MISMATCH_NUM_CHANNELS_SIGNAL_CHANNELS,
  SIGFRIED_PARAMS_MISMATCH_NUM_CHANNELS_SIGNAL_NEIGHBORS,
  SIGFRIED_PARAMS_MISMATCH_START_STOP,
  SIGFRIED_PARAMS_DELTA,
  SIGFRIED_PARAMS_HZ,
  SIGFRIED_PARAMS_BANDWIDTH,
  SIGFRIED_PARAMS_MODEL_ORDER,
  SIGFRIED_PARAMS_TREND,
  SIGFRIED_BINS2USE_MIN,
  SIGFRIED_BINS2USE_MAX,
  SIGFRIED_CH_NEIGHBORS,
  SIGFRIED_CHANNELS_MIN,
  SIGFRIED_CHANNELS_MAX,
  SIGFRIED_SPECTRAL_STEPPING,
  SIGFRIED_SEPCTRAL_SIZE,
  SIGFRIED_PARAMS_TEMPORAL_MIN_TO_LOW,
  SIGFRIED_MODEL_NUM_CHANNELS_TO_LOW,
  SIGFRIED_MODEL_MISMATCH_DIMENSION_PARAMS,
  SIGFRIED_MODEL_NUM_CENTRES_TO_LOW,
  SIGFRIED_MODEL_FILE_DOES_NOT_EXIST,
  SIGFRIED_MODEL_FILE_COULD_NOT_BE_CREATED,
  SIGFRIED_MODEL_FILE_CRC_ERROR,
  SIGFRIED_SIGFRIEDMODELLER_PARAMETERS_FILE_NOT_EXIST,
  SIGFRIED_SIGFRIEDMODELLER_PARAMETERS_FILE_KEY_NOT_EXIST
};


typedef struct SIGFRIED_MODEL_STRUCT{ 
  int             nin;
  int             ncentres;
  int             nwts;
  CVector<float>  vpriors;
  CVector<float>  vcentres;
  CVector<float>  vcovars;
  CVector<float>  vcovarsinv;
  CVector<float>  vcovarsdet;
  int             num_iteration;
  float           score_total;             
} SIGFRIED_MODEL_TYPE;


#define MAX_STRING_LENGTH 1024

typedef struct DATA2SCORE_PARAMETERS_CMD_STRUCT {

  char filename_model[MAX_STRING_LENGTH];
  char filename_cfg[MAX_STRING_LENGTH];
  char filename_in[MAX_STRING_LENGTH];
  char filename_out[MAX_STRING_LENGTH];
  bool bforce;
  bool binterp;
  bool bdebug;
  int  nskiprows;
  int  nskipcolumns;

} DATA2SCORE_PARAMETERS_CMD;

typedef struct DATA2MODEL_PARAMETERS_CMD_STRUCT {

  char filename_cfg[MAX_STRING_LENGTH];
  char filename_sig[MAX_STRING_LENGTH];
  char filename_in[MAX_STRING_LENGTH];
  char filename_out[MAX_STRING_LENGTH];
  bool bdebug;
  int  nskiprows;
  int  nskipcolumns;

} DATA2MODEL_PARAMETERS_CMD;

typedef struct SIGFRIED_PARAMETERS_CFG_STRUCT {

  // build_data
  CVector<unsigned int> vbins2use;
  CVector<unsigned int> vchannels;
  CVector<unsigned int> vch_neighbors;
  CVector<unsigned int> vtemporal;
  float                 parms[7];
  int                   spectral_size;
  int                   spectral_stepping;
  int                   filter_car;

} SIGFRIED_PARAMETERS_CFG;

typedef struct SIGFRIED_PARAMETERS_ADP_STRUCT {

  float                 learning_rate;
  float                 limit_md;
  float                 limit_neglogp;
  int                   adapt;
  int                   adapt_all;
  int                   diag;

} SIGFRIED_PARAMETERS_ADP;


/// structure to configure the SIGFRIEDmodeller
typedef struct SIGFRIEDmodeller_PARAMETERS_STRUCT {

  /// initial number of seeding classes
	int   num_class_initial;                      

  /// maximum number of classes that are handled within SIGFRIEDmodeller
  int   num_class_max;

  /// number of iterations of the CEM algorithm per split attempt
  int   num_iterations_per_split;

  /// number of iterations of the CEM algorithm per full step
  int   num_iterations_per_full_step;

  /// maximum number of iterations till tthe SIGFRIEDmodellerer quits
  int   num_iterations_max;

  /// threshold in terms of neg log(p) for a featuresample between the assigned 
  /// class and a other class that causes the SIGFRIEDmodeller#StepE to recalculate the 
  /// mahalanobis distance and the neg log likelihood for this other class
  float threshold_distance_to_recalc_score;

  /// threshold in terms of fraction of all featuresamples that have to be 
  /// reassigned in the SIGFRIEDmodeller#StepC to cause the 
  /// SIGFRIEDmodeller#CEM to do a full step
  float threshold_fraction_reassinged_to_recalc_score;

  /// #complexity_score_bic (m) of the method. 
  /// - \f$ N_p = [ N_p(cov) + N_p(center) + N_p(weight) ] \cdot n\f$
  /// - \f$ N_p = [ \frac{D (D+1)}{2} + N + 1 ] \cdot n\f$ 
  /// - \f$ AIC = N_p \cdot 2 \f$
  /// - \f$ BIC = N_p \cdot \frac{\log(N)}{2}\f$
  /// - \f$ S = (1 - m) \cdot AIC + m \cdot BIC \f$
  /// .
  float complexity_score_bic;

  /// indicates if the debug information should be shown
  int   debug_screen;

  /// the maximum float number on this platform 
  float max_float;

  /// minimum number of initial classes
  int   num_class_initial_min; 

  /// maximum number of initial classes
  int   num_class_initial_max;

  /// number of starts for each number of initial classes
  int   num_starts;

  /// startvalue for the random number generator
  int   start_random_vector;  

  /// if 0 the start_random_vector is used for intialization
  int   start_random;  

  /// if 0 there will be no split or merge in the CEM algorithm
  int   perform_recursive;

  /// if 1 there will be only diagonal covariance matrices
  int   diag;

} SIGFRIEDmodeller_PARAMETERS;



typedef int MEXPRINTF(
    const char	*fmt,	/* printf style format */
    ...				/* any additional arguments */
    );


#endif