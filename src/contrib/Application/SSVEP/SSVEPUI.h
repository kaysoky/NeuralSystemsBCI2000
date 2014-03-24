#ifndef SSVEPUI_H
#define SSVEPUI_H

#include "Environment.h"
#include "DisplayWindow.h"
#include "ApplicationBase.h"
#include "Color.h"
#include "Shapes.h"
#include "TextField.h"
#include "RadialArrow.h"

class SSVEPUI : protected Environment {
public:
    SSVEPUI(GUI::DisplayWindow&);
    ~SSVEPUI();
    
    void Initialize();
    
    enum TargetHitType {
        NOTHING_HIT, 
        YES_TARGET, 
        NO_TARGET
    };
    
    void OnStartRun();
    void OnTrialBegin();
    void ShowArrow(int);
    void ShowCross();
    void ShowText();
    void OnStopRun();
    
private:
    /**
     * Provides access to the UI window
     */
    GUI::DisplayWindow& window;
    
    /**
     * Represents the target arrows
     */
    RadialArrow **arrows;
    int numArrows;
    
    /**
     * Represents the title box
     */
    TextField* titleBox;

    /**
     * Represents the center of the screen
     */
    LineShape *horizontal;
    LineShape *vertical;
};

#endif // SSVEPUI_H
