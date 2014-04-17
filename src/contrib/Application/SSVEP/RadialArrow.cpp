#include "PCHIncludes.h"
#pragma hdrstop

#define _USE_MATH_DEFINES
#include <math.h>

#include "RadialArrow.h"
#include "BCIException.h"
#include "Shapes.h"

RadialArrow::RadialArrow(GUI::DisplayWindow& display,
                         std::string& label,
                         GUI::Point& terminus) {
    // All arrows emerge radially from the center of the screen
    GUI::Point origin = {0.5f, 0.5f};
    RGBColor arrowColor = RGBColor::White;
    
    // Interpolate the origin towards the terminus using this length parameter
    float length = Parameter("ArrowLength") / 100.0f;
    float lerp = std::pow(terminus.x - origin.x, 2) + std::pow(terminus.y - origin.y, 2);
    if (lerp == 0) {
        bcierr << "Terminus of arrow may not be the center of the screen" << std::endl;
    }
    lerp = std::min(length / std::sqrt(lerp), 1.0f);
    origin.x = origin.x * lerp + terminus.x * (1.0f - lerp);
    origin.y = origin.y * lerp + terminus.y * (1.0f - lerp);
    
    // Draw the body of the arrow
    GUI::Rect bodyRect = {origin.x, origin.y, terminus.x, terminus.y};
    body = new LineShape(display);
    body->SetObjectRect(bodyRect);
    body->SetColor(arrowColor)
         .SetLineWidth(2.5f);
         
    // Determine the characteristics of arrow flanges
    float angle = std::atan2(origin.y - terminus.y, origin.x - terminus.x);
    float theta = M_PI / 4.0f;
    float flangeLength = std::pow(terminus.x - origin.x, 2) + std::pow(terminus.y - origin.y, 2) / 2.0f;
         
    // Draw the left flange
    GUI::Rect leftRect = {flangeLength * std::cos(angle + theta) + terminus.x, 
                          flangeLength * std::sin(angle + theta) + terminus.y, 
                          terminus.x, terminus.y};
    leftFlange = new LineShape(display);
    leftFlange->SetObjectRect(leftRect);
    leftFlange->SetColor(arrowColor);
               
    // Draw the right flange
    GUI::Rect rightRect = {terminus.x, terminus.y, 
                           flangeLength * std::cos(angle - theta) + terminus.x, 
                           flangeLength * std::sin(angle - theta) + terminus.y};
    rightFlange = new LineShape(display);
    rightFlange->SetObjectRect(rightRect);
    rightFlange->SetColor(arrowColor);
    
    // Differentiate arrows by writing the frequency (Hz) over them
    GUI::Point center = {(terminus.x + origin.x) / 2.0f, (terminus.y + origin.y) / 2.0f};
    float textBoxSize = length / 4.0f;
    GUI::Rect textRect = {center.x - textBoxSize, center.y - textBoxSize, center.x + textBoxSize, center.y + textBoxSize};
    description = new TextField(display);
    description->SetText(label)
                .SetTextColor(RGBColor::Gray)
                .SetAspectRatioMode(GUI::AspectRatioModes::AdjustHeight)
                .SetTextHeight(0.5f)
                .SetColor(RGBColor::NullColor)
                .SetObjectRect(textRect);
}

RadialArrow::~RadialArrow() {
    delete description;
    delete body;
    delete leftFlange;
    delete rightFlange;
}

void RadialArrow::Show() {
    description->Show();
           body->Show();
     leftFlange->Show();
    rightFlange->Show();
}

void RadialArrow::ShowText() {
    description->Show();
}

void RadialArrow::Hide() {
    description->Hide();
           body->Hide();
     leftFlange->Hide();
    rightFlange->Hide();
}
