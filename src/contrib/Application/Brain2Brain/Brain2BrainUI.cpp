#include "PCHIncludes.h"
#pragma hdrstop

#include "Brain2BrainUI.h"
#include "BCIException.h"
#include "Shapes.h"

Brain2BrainUI::Brain2BrainUI(GUI::DisplayWindow* display) 
    : window(display) {}

Brain2BrainUI::~Brain2BrainUI() {
    delete cursor;
    delete yesTarget;
    delete noTarget;
}

void Brain2BrainUI::Initialize() {
    // Initialize the cursor to be a circle
    float cursorWidth = Parameter("CursorWidth") / 100.0f;
	GUI::Rect cursorRect = {0, 0, cursorWidth, cursorWidth * window.Width() / window.Height()};
    cursor = new EllipticShape(window, 1);
    cursor->SetObjectRect(cursorRect)
           .SetColor(RGBColor::White)
           .SetFillColor(RGBColor::White)
           .Hide();

    // On average, we need to cross half the workspace during a trial
    float feedbackDuration = Parameter("FeedbackDuration").InSampleBlocks();
    cursorSpeed = 0.5f / feedbackDuration;

    // Initialize the two targets
    float targetHeight = Parameter("TargetHeight") / 100.0f;
    RGBColor targetBorderColor = RGBColor::Gray;
    RBGColor targetFillColor = RGBColor::Blue;
    RGBColor targetTextColor = RGBColor::Black;
    float targetTextHeight = 0.5f;
    
        // Initialize the YES target
        GUI::Rect yesTargetRect = {0, 0, 1.0f, targetHeight};
        yesTarget = new RectangularShape(window);
        yesTarget->SetColor(targetBorderColor);
                  .SetFillColor(targetFillColor);
                  .SetObjectRect(yesTargetRect);
                  .Hide();
        
        yesTargetText = new TextField(window);
        yesTargetText->SetTextColor(targetTextColor)
                      .SetTextHeight(targetTextHeight)
                      .SetColor(RGBColor::NullColor)
                      .SetObjectRect(yesTargetRect)
                      .SetText("Fire")
                      .Hide();
        
        // Initialize the NO target
        GUI::Rect noTargetRect = {0, 1.0f - targetHeight, 1.0f, 1.0f};
        noTarget = new RectangularShape(window);
        noTarget->SetColor(targetBorderColor);
                 .SetFillColor(targetFillColor);
                 .SetObjectRect(noTargetRect);
                 .Hide();
        
        noTargetText = new TextField(window);
        noTargetText->SetTextColor(targetTextColor)
                     .SetTextHeight(targetTextHeight)
                     .SetColor(RGBColor::NullColor)
                     .SetObjectRect(noTargetRect)
                     .SetText("Pass")
                     .Hide();

    // Initialize the title box message
    GUI::Rect titleBoxRect = {0.1f, 0.4f, 0.9f, 0.6f};
    titleBox = new TextField(window);
    titleBox->SetTextColor(RGBColor::Lime)
             .SetTextHeight(0.8f)
             .SetColor(RGBColor::Gray)
             .SetObjectRect(titleBoxRect)
             .SetText("Timeout");
}

void Brain2BrainUI::OnStartRun() {
    titleBox.SetText(">> Get Ready! <<");
}

void Brain2BrainUI::OnTrialBegin() {
    titleBox->Hide();
    yesTarget->Show();
    yesTargetText->Show();
    noTarget->Show();
    noTargetText->Show();
}

void Brain2BrainUI::OnFeedbackBegin() {
    GUI::Point center = {0.5f, 0.5f};
    cursor->SetCenter(center)
           .Show();
}

TargetHitType Brain2BrainUI::DoFeedback(const GenericSignal& ControlSignal) {
    GUI::Point cursorPosition = cursor->Center();
    GUI::Rect cursorRect = cursor->ObjectRect();

    // Use the control signal to move up and down
    if (ControlSignal.Channels() > 0) {
        cursorPosition.y += cursorSpeed * ControlSignal(0, 0);
    }

    // Restrict cursor movement to the screen itself
    cursorPosition.y = max(cursorRect.Bottom - cursorRect.Top, 
                           min(1.0f - cursorRect.Bottom + cursorRect.Top, cursorPosition.y));

    // Update cursor position
    cursor->SetCenter(cursorPosition);
    
    // Determine if either of the targets were hit
    RGBColor hitColor = RGBColor::Red;
    if (Shape::AreaIntersection(cursor, yesTarget)) {
        yesTarget.SetColor(hitColor);
        return YES_TARGET;
    } else if (Shape::AreaIntersection(cursor, noTarget)) {
        noTarget.SetColor(hitColor);
        return NO_TARGET;
    }
    
    return NOTHING_HIT;
}

void OnFeedbackEnd() {
    cursor->Hide();
}

void OnStopRun() {
    titleBox->SetText("Timeout")
             .Show();
    yesTarget->Hide();
    yesTargetText->Hide();
    noTarget->Hide();
    noTargetText->Hide();
}
