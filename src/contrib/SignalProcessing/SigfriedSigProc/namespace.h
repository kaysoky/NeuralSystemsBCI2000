#ifndef NAMESPACE_H_INCLUDED
#define NAMESPACE_H_INCLUDED                                                   

#include "datatypes.h"

//////////////
// defaults //
//////////////


#ifdef WIN32


#else

// linux definition

#endif 

typedef void *PROGRESS(int done, int total);

typedef int  (*SET_STD_OUT)(MEXPRINTF *);

typedef int  (*SET_MALLOC)(MEXMALLOC *);

typedef int  (*SET_FREE)(MEXFREE *);

typedef int  (*SET_DEBUG)(bool);

typedef int  (*LOAD_MODEL)           ( char                         *sz_filename,
                                       CVector<SIGFRIED_MODEL_TYPE> *pvmodel,               // output 
                                       CVector<float>               *pvtrain_min,
                                       CVector<float>               *pvtrain_max,
                                       SIGFRIED_PARAMETERS_CFG      *pparameters_cfg,
                                       DATA2MODEL_PARAMETERS_CMD    *pparameters_cmd,
                                       SIGFRIEDmodeller_PARAMETERS  *pparameters_sig,
                                       CVector<char>                *pszerror,
                                       PROGRESS                     *callback_progress);


typedef int  (*SAVE_MODEL)          (char                           *sz_filename,
                                     CVector<SIGFRIED_MODEL_TYPE>   *pvmodel,               // output 
                                     CVector<float>                 *pvtrain_min,
                                     CVector<float>                 *pvtrain_max,
                                     SIGFRIED_PARAMETERS_CFG        *pparameters_cfg,
                                     DATA2MODEL_PARAMETERS_CMD      *pparameters_cmd,
                                     SIGFRIEDmodeller_PARAMETERS    *pparameters_sig,
                                     CVector<char>                  *pszerror,
                                     PROGRESS                       *callback_progress);


typedef int (*SCORE_DATA)             (CVector<unsigned int>        *pvchannels,       // input
                                       CVector<SIGFRIED_MODEL_TYPE> *pvmodel, 
                                       CVector<float>               *pvfeaturesamples,
                                       SIGFRIED_PARAMETERS_ADP      *pparameters_adp,
                                       CVector<float>               *pvscore,          // output
                                       CVector<float>               *pvneglogp,
                                       CVector<char>                *pszerror,
                                       PROGRESS                     *callback_progress);



typedef int  (*SCORE_DATA_MEX)      (int                       nchannel,          // input
                                     CVector<float>           *pvclass_weight,
                                     CVector<float>           *pvcentres,
                                     CVector<float>           *pvcovars,
                                     CVector<float>           *pvcovarsinv,
                                     CVector<float>           *pvcovarsdet,
                                     CVector<float>           *pvfeaturesamples,
                                     SIGFRIED_PARAMETERS_ADP  *pparameters_adp,
                                     CVector<float>           *pvscore,         // output
                                     CVector<float>           *pvneglogp,
                                     int                      *pnum_adapt_total);


typedef int (*BUILD_MODEL_MEX)   (CVector<float>              *pvfeaturesamples,     // input
                                  int                          nchannel,
                                  CVector<float>              *pvclass_weight,       // output
                                  CVector<float>              *pvcentres,
                                  CVector<float>              *pvcovars,
                                  int                         *pnum_classes,
                                  int                         *pnum_iteration,
                                  float                       *pscore_total,
                                  SIGFRIEDmodeller_PARAMETERS *parameters_sig);


typedef int (*BUILD_MODEL)(CVector<float>               *pvfeaturesamples,
                           CVector<SIGFRIED_MODEL_TYPE> *pvmodel,
                           SIGFRIEDmodeller_PARAMETERS  *parameters_sig,
                           CVector<char>                *pszerror,
                           PROGRESS                     *callback_progress);



typedef int (*NORMALIZE_DATA)       (CVector<float>     *pvfeaturesamples,      // input
                                     CVector<float>     *pvtrain_min,
                                     CVector<float>     *pvtrain_max,
                                     CVector<float>     *pvfeaturesamples_out,  // output
                                     CVector<char>      *pszerror,
                                     PROGRESS           *callback_progress);

typedef int (*BUILD_DATA_FREQUENCY_SAMPLE_INIT)(int                      index_instance,
                                                SIGFRIED_PARAMETERS_CFG *pparameters_cfg);


typedef int (*BUILD_DATA_FREQUENCY_SAMPLE)(int               index_instance,
                                           CVector<float>   *psignal,             // input
                                           CVector<float>   *pfeaturesamples);    // output


typedef int (*BUILD_DATA_FREQUENCY)(CVector<float>          *psignal,              // input
                                    CVector<unsigned int>   *peventcodes,
                                    unsigned int             mask,
                                    SIGFRIED_PARAMETERS_CFG *pparameters_cfg,
                                    CVector<float>          *pfeaturesamples,      // output
                                    CVector<unsigned int>   *peventcodes_out,
                                    CVector<float>          *ptrain_min,
                                    CVector<float>          *ptrain_max,
                                    CVector<char>           *pszerror,
                                    PROGRESS                *callback_progress);

typedef int (*GET_FEATURE_SIZE)(  CVector<float>            *psignal,              // input
                                  CVector<unsigned int>     *peventcodes,
                                  unsigned int               mask,
                                  SIGFRIED_PARAMETERS_CFG   *pparameters_cfg,
                                  CVector<int>              *pfeature_size,      // output
                                  CVector<int>              *peventcodes_out_size,
                 								  CVector<int>              *ptrain_size,
                                  unsigned int              *pnum_derivations);


typedef int (*GET_FEATURE_SIZE_SAMPLE)(int                   index_instance,
                                       CVector<int>          feature_size);      // output

typedef int (*CHECK_PARAMETERS_PARAMS)      (CVector<unsigned int> *pbins2use,
                                             CVector<float>        *pparms,
                                             CVector<char>         *pszerror);


typedef int (*CHECK_PARAMETERS_CHANNELS)      (CVector<unsigned int>  *pchannels,
                                               CVector<unsigned int>  *pch_neighbors,
                                               CVector<char>          *pszerror);

typedef int (*CHECK_PARAMETERS_MODEL)(CVector<SIGFRIED_MODEL_TYPE>    *pvmodel,
                                      CVector<unsigned int>           *pbins2use,
                                      CVector<unsigned int>           *pchannels,
                                      CVector<unsigned int>           *pch_neighbors,
                                      CVector<unsigned int>           *ptemporal,
                                      CVector<char>                   *pszerror);

typedef int (*GET_SIGFRIEDMODELLER_PRM)(char                          *szconfigurationfilename,  // input
                                        SIGFRIEDmodeller_PARAMETERS   *pparameters_sig,          // output
                                        CVector<char>                 *pszerror); 


typedef int (*SIGFRIED_BUILD_MODEL)(CVector<float>            *vsignal,              // input
                                    SIGFRIED_PARAMETERS_CFG   *parameters_cfg,
                                    char                      *szconfigurationfilename,
                                    char                      *szmodelfilename,
                                    CVector<char>             *pszerror,
                                    PROGRESS                  *callback_progress);

typedef int (*SIGFRIED_SCORE_DATA) (CVector<float>            *vsignal,              // input
                                    char                      *szmodelfilename,
                                    CVector<float>            *vscore_out,
                                    CVector<char>             *pszerror,
                                    PROGRESS                  *callback_progress);




#endif
