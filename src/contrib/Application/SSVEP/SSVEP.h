#ifndef SSVEP_H
#define SSVEP_H

#include "MongooseFeedbackTask.h"
#include "ApplicationWindow.h"
#include "BlockRandSeq.h"
#include "mongoose.h"
#include "SSVEPUI.h"
#include <vector>

class SSVEPFeedbackTask : public MongooseFeedbackTask {
public:
    SSVEPFeedbackTask();
    ~SSVEPFeedbackTask();
    
private:
    // Startup events
    virtual void OnPreflight(const SignalProperties&) const;
    virtual void OnInitialize(const SignalProperties&);
    virtual void OnStartRun();
    virtual void DoPreRun(const GenericSignal&, bool& doProgress);

    // Trial Loop
    virtual void OnTrialBegin();
    virtual void DoPreFeedback(const GenericSignal&, bool& doProgress) { doProgress = true; };
    virtual void OnFeedbackBegin() {};
    virtual void DoFeedback(const GenericSignal&, bool& doProgress);
    virtual void OnFeedbackEnd() {};
    virtual void DoPostFeedback(const GenericSignal&, bool& doProgress) { doProgress = true; };
    virtual void OnTrialEnd();
    virtual void DoITI(const GenericSignal&, bool&);

    // Cleanup events
    virtual void OnStopRun();
    virtual void OnHalt() {};

    /////////////////////////
    // Brain2Brain Objects //
    /////////////////////////

    // Graphics objects
    ApplicationWindow &window;
    SSVEPUI *SSVEPGUI;

    int runCount,
        trialCount;
    
    virtual void HandleTrialStartRequest(std::string, int&);
    virtual bool HandleTrialStatusRequest(struct mg_connection *);
    virtual void HandleAnswerUpdate(std::string);
        
    /*
     * Toggles specific application code flow depending on which one is active
     */
    enum StimulusMode {
        Training, 
        Classification
    };
    StimulusMode mode;
    
    /*
     * Classification mode only
     * Read in the training file
     *   and puts the data into a vector (distributions)
     */
    void ParseTrainingFile();
    
    /*
     * Training mode only
     * Calculates the mean and variance of the raw data (distributions)
     *   and writes it to the training file
     */
    void SaveTrainingFile();
    
    /*
     * Holds the information necessary for the SSVEP's Naive Bayes classifier
     */
    struct NormalData {
        int frequency;
        double mean;
        double variance;
        
        /*
         * Training mode only
         * Holds raw training data
         */
        std::vector<double> raw;
        
        /*
         * Classification mode only
         * Holds the prior Bayesian belief
         */
        double prior;
    };
    std::vector<NormalData> distributions;
    
    /*
     * Classification mode only
     * Holds the classification threshold
     */
    double classificationThreshold;
    bool classificationMade;
    
    /*
     * Training mode only
     * Randomizes the order of arrows
     */
    BlockRandSeq arrowSequence;
    int currentTrainingType;
};

#endif // SSVEP_H

