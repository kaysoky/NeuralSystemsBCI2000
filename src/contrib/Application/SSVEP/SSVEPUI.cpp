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
    delete horizontal;
    delete vertical;
    delete questionBox;
    delete answerBox;
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

    // Initialize the center "+" mark
    RGBColor centerColor = RGBColor::White;
    float lineWidth = 2.5f;
    GUI::Rect horizontalLine = {0.4f, 0.5f, 0.6f, 0.5f};
    horizontal = new LineShape(window);
    horizontal->SetObjectRect(horizontalLine);
    horizontal->SetColor(centerColor)
               .SetLineWidth(lineWidth);
    
    GUI::Rect verticalLine = {0.5f, 0.4f, 0.5f, 0.6f};
    vertical = new LineShape(window);
    vertical->SetObjectRect(verticalLine);
    vertical->SetColor(centerColor)
             .SetLineWidth(lineWidth);
             
    // Initialize the optional text fields
    RGBColor textColor = RGBColor::White;
    GUI::Rect questionBoxRect = {0.1f, 0.1f, 0.9f, 0.3f};
    questionBox = new TextField(window);
    questionBox->SetText("")
                .SetColor(textColor)
                .SetObjectRect(questionBoxRect);
                
    GUI::Rect answerBoxRect = {0.1f, 0.7f, 0.9f, 0.9f};
    answerBox = new TextField(window);
    answerBox->SetText("")
              .SetColor(textColor)
              .SetObjectRect(answerBoxRect);
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

    // Hide the cross
    horizontal->Hide();
    vertical->Hide();
}

void SSVEPUI::ShowText() {
    // Hide the cross
    horizontal->Hide();
    vertical->Hide();

    // Show just the text, not the arrows
    for (int i = 0; i < numArrows; i++) {
        arrows[i]->Hide();
        arrows[i]->ShowText();
    }
    
    questionBox->Show();
    answerBox->Show();
}

void SSVEPUI::ShowCross() {
    // Hide the arrows
    for (int i = 0; i < numArrows; i++) {
        arrows[i]->Hide();
    }

    horizontal->Show();
    vertical->Show();
}

void SSVEPUI::OnStopRun() {
    titleBox->SetText("Timeout")
             .Show();

    for (int i = 0; i < numArrows; i++) {
        arrows[i]->Hide();
    }
    
    questionBox->Hide();
    answerBox->Hide();
}

void SSVEPUI::SetQuestion(std::string data) {
    questionBox->SetText(data);
}

void SSVEPUI::SetAnswer(std::string data) {
    answerBox->SetText(data);
}
