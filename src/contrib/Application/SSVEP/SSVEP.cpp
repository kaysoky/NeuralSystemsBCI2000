#include "PCHIncludes.h"
#pragma hdrstop

#include "SSVEP.h"
#include "FileUtils.h"

#include <stdio.h>
#include <cmath>
#include <fstream>

RegisterFilter( SSVEPFeedbackTask, 3 );

SSVEPFeedbackTask::SSVEPFeedbackTask()
    : SSVEPGUI(NULL),
      window(Window()),
      runCount(0),
      arrowSequence(RandomNumberGenerator) {

    // Note: See MongooseTask.cpp for more parameters and states

    BEGIN_PARAMETER_DEFINITIONS
    "Application:SSVEP matrix Arrows= "
        " 2 " // rows
        " [Frequency X Y Label] " // columns
        " 12 10 50 Yes"
        " 17 90 50 No"
        " // Frequency of input expected for each target and their position in percentage coordinates.  Note: the first row is the YES arrow",
    "Application:SSVEP float ArrowLength= 10 % 0 70 "
        " // Length of an arrow in percent of screen dimensions",
    "Application:SSVEP int StimulusMode= 0 0 0 1"
        " // Training or classification: 0 Training, 1 Classification (enumeration)",
    "Application:SSVEP string TrainingFile= ../Data/SSVEP/Training.tsv"
        " // File to store/read the result of training (inputfile)",
    "Application:SSVEP float ClassificationThreshold= 0.95 % 0 1 "
        " // Probability threshold at which some particular classification is made"
    END_PARAMETER_DEFINITIONS
}

SSVEPFeedbackTask::~SSVEPFeedbackTask() {
	delete SSVEPGUI;
}

void SSVEPFeedbackTask::OnPreflight(const SignalProperties& Input) const {
    if (Parameter("Arrows")->NumRows() <= 1) {
        bcierr << "At least two target frequencies must be specified" << std::endl;
    }
    if (Parameter("Arrows")->NumColumns() != 4) {
        bcierr << "Target matrix must have 4 columns "
               << "corresponding to the target frequency, "
               << "(X, Y) positions, "
               << "and label" << std::endl;
    }
    Parameter("ArrowLength");

    for (int i = 0; i < Parameter("Arrows")->NumRows(); i++) {
        int frequency = Parameter("Arrows")(i, 0);
        if (Input.Elements() <= frequency) {
            bcierr << "Input signal does not contain the target frequency ("
                   << frequency << " Hz)" << std::endl;
        }
    }

    int modeParam = Parameter("StimulusMode");
    Parameter("TrainingFile");
    if (static_cast<StimulusMode>(modeParam) == Classification
            && !FileUtils::Exists(std::string(Parameter("TrainingFile")))) {
        bcierr << "Specified training file (" << Parameter("TrainingFile")
               << ") does not exist" << std::endl;
    }
    Parameter("ClassificationThreshold");

    CheckServerParameters(Input);
}

void SSVEPFeedbackTask::OnInitialize(const SignalProperties& Input) {
    InitializeServer(Input, Parameter("Arrows")->NumRows());
    state_lock->Acquire();

    // Save the mode
    int modeParam = Parameter("StimulusMode");
    mode = static_cast<StimulusMode>(modeParam);

    // Remove any existing data
    distributions.clear();
    for (int i = 0; i < Parameter("Arrows")->NumRows(); i++) {
        NormalData empty;
        empty.frequency = Parameter("Arrows")(i, 0);
        distributions.push_back(empty);
    }

    // Read the training data
    if (mode == Classification) {
        ParseTrainingFile();
        classificationThreshold = Parameter("ClassificationThreshold");
    }

    // Initialize the arrow randomizer
    size_t numTrials = 100; // Arbitrary default
    if (!std::string(Parameter("NumberOfTrials")).empty()) {
        numTrials = Parameter("NumberOfTrials");
    }
    arrowSequence.SetBlockSize(numTrials);

    // Reset the GUI
    delete SSVEPGUI;
    SSVEPGUI = new SSVEPUI(window);
    SSVEPGUI->Initialize();

    state_lock->Release();
}

void SSVEPFeedbackTask::ParseTrainingFile() {
    std::string trainingFilename(Parameter("TrainingFile"));
    std::fstream trainingFile(trainingFilename.c_str(), std::fstream::in);

    // The first line contains header info, so skip it
    std::string line;
    std::getline(trainingFile, line);

    // Read the .tsv
    for (int i = 0; i < Parameter("Arrows")->NumRows(); i++) {
        if (trainingFile.eof()) {
            bcierr << "Unexpected end of training file: "
                   << "Expected " << Parameter("Arrows")->NumRows() << " lines, "
                   << "Got " << i << std::endl;
        }
        if (trainingFile.fail()) {
            bcierr << "Failed to read training file" << std::endl;
        }

        NormalData parsed;

        // Parse the frequency
        std::getline(trainingFile, line, '\t');
        parsed.frequency = std::atoi(line.c_str());
        if (parsed.frequency != Parameter("Arrows")(i, 0)) {
            bcierr << "Training file does not match configuration: "
                   << "Expected line " << (i + 2) << " of training data to be "
                   << Parameter("Arrows")(i, 0) << " Hz, "
                   << "Got " << parsed.frequency << " Hz" << std::endl;
        }

        // Parse the mean
        std::getline(trainingFile, line, '\t');
        parsed.mean = std::atof(line.c_str());

        // Parse the standard variance
        std::getline(trainingFile, line);
        parsed.variance = std::atof(line.c_str());

        // Save the data
        distributions[i] = parsed;
    }
    trainingFile.close();
}

void SSVEPFeedbackTask::OnStartRun() {
    // Reset state of the game
    MongooseOnStartRun();

    // Reset or increment some counters
    runCount++;
    trialCount = 0;

    AppLog << "Run #" << runCount << " started" << std::endl;
    SSVEPGUI->OnStartRun();
}

void SSVEPFeedbackTask::DoPreRun(const GenericSignal&, bool& doProgress) {
    // Let the Feedback task handle the timing for training
    if (mode == Training) {
        return;
    }

    // Wait for the start signal
    doProgress = false;

    state_lock->Acquire();
    if (lastClientPost == START_TRIAL) {
        doProgress = true;
        lastClientPost = CONTINUE;
    }
    state_lock->Release();
}

void SSVEPFeedbackTask::OnTrialBegin() {
    // Increment the trial count
    trialCount++;

    SSVEPGUI->OnTrialBegin();
    AppLog << "Trial #" << trialCount << " => ";
    
    // Reset trial-specific 20 questions state
    State("TargetHitCode") = 0;
    state_lock->Acquire();
    classificationMade = false;

    // Mode-specific initialization
    unsigned int i;
    switch (mode) {
    case Training:
        // Determine the training frequency
        currentTrainingType = arrowSequence.NextElement() % distributions.size();
        AppLog << Parameter("Arrows")(currentTrainingType, 0) << " Hz" << std::endl;
        SSVEPGUI->ShowArrow(currentTrainingType);
        break;
    case Classification:
        // Reset the priors
        for (i = 0; i < distributions.size(); i++) {
            distributions[i].prior = 1.0 / distributions.size();
        }

        AppLog << "Classifying..." << std::endl;
        SSVEPGUI->ShowText();
        break;
    default:
        bcierr << "Unknown stimulus mode" << std::endl;
        break;
    }
    state_lock->Release();
}

// Before C++ 11, the erf(...) function did not exist
#if __cplusplus <= 199711L
namespace std {
    /*
     * Taken from: http://www.johndcook.com/cpp_erf.html
     */
    static double erf(double x) {
        // constants
        double a1 =  0.254829592;
        double a2 = -0.284496736;
        double a3 =  1.421413741;
        double a4 = -1.453152027;
        double a5 =  1.061405429;
        double p  =  0.3275911;

        // Save the sign of x
        int sign = 1;
        if (x < 0)
            sign = -1;
        x = fabs(x);

        // A&S formula 7.1.26
        double t = 1.0/(1.0 + p*x);
        double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

        return sign*y;
    }
}
#endif

/*
 * Calculates the cumulative probability of the given value
 *   in the given normal distribution
 * Note: Should be divided by 2,
 *       but this step is not necessary due to normalization of results
 */
static double NormalCDF(double value, double mean, double variance) {
    return 1.0 + std::erf((value - mean) / std::sqrt(2.0 * variance));
}

void SSVEPFeedbackTask::DoFeedback(const GenericSignal& ControlSignal, bool& doProgress) {
    unsigned int i;
    double sum;
    switch (mode) {
    case Training:
        // Save the data
        distributions[currentTrainingType].raw
                .push_back(ControlSignal(0, Parameter("Arrows")(currentTrainingType, 0)));
        break;
    case Classification:
            doProgress = false;
            state_lock->Acquire();

            // Iterate the Naive Bayes priors
            sum = 0.0;
            for (i = 0; i < distributions.size(); i++) {
                distributions[i].prior *=
                        NormalCDF(ControlSignal(0, distributions[i].frequency),
                                  distributions[i].mean,
                                  distributions[i].variance);
                sum += distributions[i].prior;
            }

            // Normalize the priors
            for (i = 0; i < distributions.size(); i++) {
                distributions[i].prior /= sum;
                if (distributions[i].prior >= classificationThreshold) {
                    State("TargetHitCode") = i;
                }
            }

            // Check for the stop signal
            if (lastClientPost == STOP_TRIAL || classificationMade) {
                doProgress = true;
                lastClientPost = CONTINUE;
            }
            state_lock->Release();
        break;
    default:
        bcierr << "Unknown stimulus mode" << std::endl;
        break;
    }
}

void SSVEPFeedbackTask::OnTrialEnd() {
    SSVEPGUI->ShowCross();
}

void SSVEPFeedbackTask::DoITI(const GenericSignal&, bool& doProgress) {
    // Let the Feedback task handle the timing for training
    if (mode == Training) {
        return;
    }

    doProgress = false;

    // Wait for the start signal
	state_lock->Acquire();
    if (lastClientPost == START_TRIAL) {
        doProgress = true;
		lastClientPost = CONTINUE;
    }
	state_lock->Release();
}

void SSVEPFeedbackTask::OnStopRun() {
    // Save the data to the training file
    if (mode == Training) {
        SaveTrainingFile();
    }

    AppLog << "Run " << runCount << " finished: "
           << trialCount << " trial(s)" << std::endl;

    MongooseOnStopRun();
    SSVEPGUI->OnStopRun();
}

void SSVEPFeedbackTask::SaveTrainingFile() {
    std::string trainingFilename(Parameter("TrainingFile"));
    std::fstream trainingFile(trainingFilename.c_str(), std::fstream::out | std::fstream::trunc);

    trainingFile << "Frequency\tMean\tVariance" << std::endl;
    for (unsigned int i = 0; i < distributions.size(); i++) {
        // Calculate the mean
        double sum = 0.0;
        for (unsigned int j = 0; j < distributions[i].raw.size(); j++) {
            sum += distributions[i].raw[j];
        }
        distributions[i].mean = sum / distributions[i].raw.size();

        // Calculate the variance
        sum = 0.0;
        for (unsigned int j = 0; j < distributions[i].raw.size(); j++) {
            sum += std::pow(distributions[i].mean - distributions[i].raw[j], 2);
        }
        distributions[i].variance = sum / distributions[i].raw.size();

        // Write the data to the file
        trainingFile << distributions[i].frequency << "\t"
                     << distributions[i].mean      << "\t"
                     << distributions[i].variance  << std::endl;
    }
    trainingFile.close();
}

bool SSVEPFeedbackTask::HandleTrialStatusRequest(struct mg_connection *conn) {
    if (mode == Classification) {
        if (classificationMade) {
            return false;
        }

        // Classify!
        for (unsigned int i = 0; i < distributions.size(); i++) {
            if (distributions[i].prior >= classificationThreshold) {
                mg_send_status(conn, 200);
                mg_send_header(conn, "Content-Type", "text/plain");

                // By default, the first arrow will be the yes target
                // Note: This will likely need to be changed later on
                AppLog << "Classification made: ";
                if (i == 0) {
                    mg_printf_data(conn, "YES");
                    AppLog << "YES";
                } else {
                    mg_printf_data(conn, "NO");
                    AppLog << "NO (" << i << ")";
                }
                AppLog << std::endl;

                classificationMade = true;
                return true;
            }
        }
    }

    return false;
}

void SSVEPFeedbackTask::HandleTrialStartRequest(std::string data, int &) {
    SSVEPGUI->SetQuestion(data);
}

void SSVEPFeedbackTask::HandleAnswerUpdate(std::string data) {
    SSVEPGUI->SetAnswer(data);
}
