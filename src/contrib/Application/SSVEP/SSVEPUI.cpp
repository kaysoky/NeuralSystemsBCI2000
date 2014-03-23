#include "PCHIncludes.h"
#pragma hdrstop

#include <math.h>

#include "SSVEPUI.h"
#include "BCIException.h"
#include "Shapes.h"

SSVEPUI::SSVEPUI(GUI::DisplayWindow& display) 
    : window(display) {}

SSVEPUI::~SSVEPUI() {
    for (int i = 0; i < numArrows; i++) {
        delete arrows[i];
    }
    delete arrows;
    delete titleBox;
}

void SSVEPUI::Initialize() {
    // Initialize the arrows
    ParamRef matrix = Parameter("Arrows");
    numArrows = matrix->NumRows();
    arrows = new RadialArrow*[numArrows];
    for (int i = 0; i < matrix->NumRows(); i++) {
        GUI::Point terminus = { matrix(i, 1) / 100.0f, matrix(i, 2) / 100.0f };
        arrows[i] = new RadialArrow(window, matrix(i, 0), terminus);
        arrows[i]->Hide();
    }

    // Initialize the title box message
    GUI::Rect titleBoxRect = {0.1f, 0.4f, 0.9f, 0.6f};
    titleBox = new TextField(window);
    titleBox->SetText("Timeout")
		     .SetTextColor(RGBColor::Lime)
             .SetTextHeight(0.8f)
             .SetColor(RGBColor::Gray)
             .SetObjectRect(titleBoxRect);
}

void SSVEPUI::OnStartRun() {
    titleBox->SetText(">> Get Ready! <<");
}

void SSVEPUI::OnTrialBegin() {
    titleBox->Hide();
}

void SSVEPUI::ShowArrow(int num) {
    if (num >= numArrows || num < 0) {
        throw "Index out of bounds";
    }
    
    // Only show one arrow at once
    for (int i = 0; i < numArrows; i++) {
        arrows[i]->Hide();
    }
    arrows[num]->Show();
}

void SSVEPUI::OnStopRun() {
    titleBox->SetText("Timeout")
             .Show();

    for (int i = 0; i < numArrows; i++) {
        arrows[i]->Hide();
    }
}
