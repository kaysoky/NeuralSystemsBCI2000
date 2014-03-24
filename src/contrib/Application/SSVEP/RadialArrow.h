#ifndef RADIAL_ARROW_H
#define RADIAL_ARROW_H

#include "Environment.h"
#include "DisplayWindow.h"
#include "ApplicationBase.h"
#include "Color.h"
#include "Shapes.h"
#include "TextField.h"

class RadialArrow : protected Environment {
public:
    RadialArrow(GUI::DisplayWindow&, int, GUI::Point&);
    ~RadialArrow();
    
    void Show();
    void ShowText();
    void Hide();
    
private:
    TextField* description;
    LineShape* body;
    LineShape* leftFlange;
    LineShape* rightFlange;
};

#endif // RADIAL_ARROW_H
