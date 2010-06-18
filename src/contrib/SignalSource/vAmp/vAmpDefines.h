#ifdef FIRSTAMP_EXPORTS
/*! FIRSTAMP_API functions as being exported from a DLL */
#define FIRSTAMP_API __declspec(dllexport)
#else
/*! FIRSTAMP_API functions as being imported from a DLL */
#define FIRSTAMP_API extern "C" __declspec(dllimport)
#endif

/*! Error codes */
#define FA_ERR_OK (0) /*!< Success (no errors) */
#define FA_ERR_ID (-1) /*!< Invalid ID (such ID not present now) */
#define FA_ERR_FAIL (-2) /*!< Function fail (internal error) */
#define FA_ERR_HANDLE (-3) /*!< Invalid handle (driver already used) */
#define FA_ERR_PARAM (-4) /*!< Invalid function parameter(s) */

/*! Invalid device ID */
#define FA_ID_INVALID (0)

/*! FirstAmp models */
#define FA_MODEL_16 (1) /*!< 16+1 main and 2 auxiliary channels */
#define FA_MODEL_8 (0) /*!< 8+1 main and 2 auxiliary channels */

/*! FirstAmp model 16 channels count */
#define FA_MODEL_16_CHANNELS_MAIN (16 + 1) /*!< 16 (EEG) + 1 (REF) channels */
#define FA_MODEL_16_CHANNELS_AUX (2) /*!< 2 auxiliary channels */

/*! FirstAmp model 8 channels count */
#define FA_MODEL_8_CHANNELS_MAIN (8 + 1) /*!< 8 (EEG) + 1 (REF) channels */
#define FA_MODEL_8_CHANNELS_AUX (2) /*!< 2 auxiliary channels */

/*! FirstAmp model 19/8 channels count in 20 kHz mode */
#define FA_MODE_20_KHZ_CHANNELS_MAIN (4) /*!< 4 (EEG) channels */
#define FA_MODE_20_KHZ_CHANNELS_AUX (0) /*!< 0 auxiliary channels */

/*! FirstAmp status bits (t_faDataModelX.Status) in data type */
#define FA_STATUS_BIT_IN_COUNT (9) /*!< Count of digital inputs */
#define FA_STATUS_BIT_IN_SHIFT (0) /*!< Position of digital inputs bits from LSB */
#define FA_STATUS_BIT_OUT_COUNT (1) /*!< Count of digital outputs */
#define FA_STATUS_BIT_OUT_SHIFT (9) /*!< Position of digital outputs bits from LSB */
/*! Total status bits count */
#define FA_STATUS_BITS_COUNT (FA_STATUS_BIT_IN_COUNT + FA_STATUS_BIT_OUT_COUNT)

/*! Size of user data area, bytes */
#define FA_USER_DATA_SIZE (512) /* do not change this */

#define MAX_ALLOWED_DEVICES			1//#define MAX_ALLOWED_DEVICES			4



/****** Acquisition types (nAcquisitionType), used by IAmplifierControl::Start(): ******/
#define IAC_AT_IMPEDANCE			0		// Impedance check
#define IAC_AT_DATA					1		// Standard data aquisition
#define IAC_AT_CALIBRATION			2		// Calibration


#define DEVICE_GET_DATA_INTERVAL	20		// 20 >> 40ms data service interval(Test Software).
											// If you change this, please also change
											// the ringbuffer size.
											// NOTE: 20ms data service block because of correcting trigger 
											// and driver doesn't let to fetch data to much at one time.

#define DEVICE_SERVE_TIMEOUT		50000	// Idle time 5 sec.
#define DEVICE_BUFTIME				32		// 4 seconds of local ring buffer.

#define MIN_SAMPLING_RATE			100		// Min. Sampling Rate 100 Hz because 
											// of 20 x 100 / 1000 = 2 PointsPerBlock.

#define BIT(x)						(1 << x)

