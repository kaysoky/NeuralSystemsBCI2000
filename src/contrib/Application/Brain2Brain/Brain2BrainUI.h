#ifndef BRAIN2BRAINUI_H
#define BRAIN2BRAINUI_H

#include "Environment.h"
#include "DisplayWindow.h"
#include "ApplicationBase.h"
#include "Color.h"
#include "Shapes.h"
#include "TextField.h"

class Brain2BrainUI : protected Environment {
public:
    Brain2BrainUI(GUI::DisplayWindow&);
    ~Brain2BrainUI();
    
    void Initialize();
    
    enum TargetHitType {
        NOTHING_HIT, 
        YES_TARGET, 
        NO_TARGET
    };
    
    void OnStartRun();
    void OnFeedbackBegin();
    TargetHitType DoFeedback(const GenericSignal&);
    void OnFeedbackEnd();
    void OnStopRun();
    
    void SetQuestion(std::string);
    void SetAnswer(std::string);
    void ShowQuestion();
    void HideQuestion();
    
    /*
     * Returns the closer of the two targets, relative to the cursor
     */
    TargetHitType GetClosestTarget();
    
private:
    /**
     * Provides access to the UI window
     */
    GUI::DisplayWindow& window;
    
    /**
     * Represents the cursor
     */
    EllipticShape* cursor;
    float cursorSpeed;
    
    /**
     * Represents the YES target
     */
    RectangularShape* yesTarget;
    TextField* yesTargetText;
    
    /**
     * Represents the NO target
     */
    RectangularShape* noTarget;
    TextField* noTargetText;

    /**
     * Represents the title box
     */
    TextField* titleBox;
    
    /**
     * Represents the optional text fields
     */
    TextField *questionBox;
    TextField *answerBox;


    /**
     * Amount of time the cursor has stayed over a target
     * (In sample blocks)
     */
    int dwellTime;
};

#endif // BRAIN2BRAINUI_H
