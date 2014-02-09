#include "PCHIncludes.h"
#pragma hdrstop

#include "DFBuildScene2D.h"
#include "GraphDisplay.h"
#include "BCIException.h"
#include "GradientEllipticShape.h"
using namespace std;

DFBuildScene2D::DFBuildScene2D(GUI::DisplayWindow& inDisplay) : 
    mDisplay(inDisplay),
    mScalingX(1.0),
    mScalingY(1.0),
    mCursorZ(0.0),
    mpCursor(NULL),
    mpBoundary(NULL) {}

DFBuildScene2D::~DFBuildScene2D() {
    ClearObjects();
}

DFBuildScene2D&
DFBuildScene2D::Initialize() {
    ClearObjects();

    // Square out the display via scaling
    float width = mDisplay.Width();
    float height = mDisplay.Height();
    if (width > height) {
        mScalingX = height / width;
        mScalingY = -1.0;
    } else {
        mScalingX = 1.0;
        mScalingY = -width / height;
    }

    // Draw a border around the edge
    RGBColor boundaryColor(Parameter("WorkspaceBoundaryColor"));
    if (boundaryColor != RGBColor(RGBColor::NullColor)) {
        mpBoundary = new RectangularShape(mDisplay);
        mpBoundary->SetColor(boundaryColor)
                   .SetFillColor(RGBColor::NullColor);
		GUI::Point boundaryDiag = {100, 100};
        SceneToObjectCoords(boundaryDiag, vector);
		GUI::Rect boundaryRect = {0, 0, fabs(boundaryDiag.x), fabs(boundaryDiag.y)};
        mpBoundary->SetObjectRect(boundaryRect);
		GUI::Point boundaryCenter = {50, 50};
        SceneToObjectCoords(boundaryCenter, point);
        mpBoundary->SetCenter(boundaryCenter);
    }
    
    // Draw the cursor
    mpCursor = new EllipticShape(mDisplay, 1);
    float cursorWidth = Parameter("CursorWidth");
	GUI::Point cursorDiag = {cursorWidth, cursorWidth};
    SceneToObjectCoords(cursorDiag, vector);
	GUI::Rect cursorRect = {0, 0, fabs(cursorDiag.x), fabs(cursorDiag.y)};
    mpCursor->SetObjectRect(cursorRect);
    mCursorZ = 0;

    // Use an enum to index into the various parameters
    enum {x, y, dx, dy};
    
    ParamRef CursorPos = Parameter("CursorPos");
    SetCursorPosition(CursorPos(x), CursorPos(y), 0);
    mpCursor->Hide();

    // Position all the targets
    ParamRef Targets = Parameter("Targets");
    for (int i = 0; i < Parameter("Targets")->NumRows(); ++i) {
        // Determine the target's dimensions
        EllipticShape* pTarget = new GradientEllipticShape(mDisplay);
        GUI::Point targetDiag = {Targets(i, dx), Targets(i, dy)};
        SceneToObjectCoords(targetDiag, vector);
        
        // Set the target's size
        GUI::Rect targetRect = {0, 0, fabs(targetDiag.x), fabs(targetDiag.y)};
        pTarget->SetObjectRect(targetRect);
        
        // Set the target's origin
        GUI::Point targetCenter = {Targets(i, x), Targets(i, y)};
        SceneToObjectCoords(targetCenter, point);
        pTarget->SetCenter(targetCenter);
        
        // Hide and save the target
        pTarget->Hide();
        mTargets.push_back(pTarget);
    }
    return *this;
}

float
DFBuildScene2D::CursorRadius() const {
    GUI::Rect r = mpCursor->ObjectRect();
    GUI::Point p = {r.right - r.left, r.bottom - r.top};
    ObjectToSceneCoords(p, vector);
    return max(fabs(p.x), fabs(p.y)) / 2.;
}

DFBuildScene2D&
DFBuildScene2D::SetCursorPosition(float inX, float inY, float inZ) {
    mCursorZ = inZ;
    GUI::Point center = {inX, inY};
    SceneToObjectCoords(center, point);
    mpCursor->SetCenter(center);
    return *this;
}

float
DFBuildScene2D::CursorXPosition() const {
    GUI::Point position = mpCursor->Center();
    ObjectToSceneCoords(position, point);
    return position.x;
}

float
DFBuildScene2D::CursorYPosition() const {
    GUI::Point position = mpCursor->Center();
    ObjectToSceneCoords(position, point);
    return position.y;
}

float
DFBuildScene2D::CursorZPosition() const {
    return mCursorZ;
}


DFBuildScene2D&
DFBuildScene2D::SetCursorVisible(bool inVisible) {
    inVisible ? mpCursor->Show() : mpCursor->Hide();
    return *this;
}


DFBuildScene2D&
DFBuildScene2D::SetCursorColor(RGBColor inColor) {
    mpCursor->SetColor(inColor).SetFillColor(inColor);
    return *this;
}

int
DFBuildScene2D::NumTargets() const {
    return static_cast<int>(mTargets.size());
}

bool
DFBuildScene2D::TargetHit(int inIdx) const {
    return Shape::AreaIntersection(*mpCursor, *mTargets.at(inIdx));
}

float
DFBuildScene2D::CursorTargetDistance(int inIdx) const {
    GUI::Point targetCenter = mTargets.at(inIdx)->Center(),
            cursorCenter = mpCursor->Center();
    ObjectToSceneCoords(targetCenter, point);
    ObjectToSceneCoords(cursorCenter, point);
    GUI::Point diff = targetCenter - cursorCenter;
    return ::sqrt(diff.x * diff.x + diff.y * diff.y);
}

DFBuildScene2D&
DFBuildScene2D::SetTargetVisible(bool inVisible, int inIdx) {
    inVisible ? mTargets.at(inIdx)->Show() : mTargets.at(inIdx)->Hide();
    return *this;
}

DFBuildScene2D&
DFBuildScene2D::SetTargetColor(RGBColor inColor, int inIdx) {
    mTargets.at(inIdx)->SetColor(inColor)
                       .SetFillColor(inColor);
    return *this;
}

void
DFBuildScene2D::ClearObjects() {
    delete mpBoundary;
    mpBoundary = NULL;
    delete mpCursor;
    mpCursor = NULL;
    for (size_t i = 0; i < mTargets.size(); ++i) {
        delete mTargets[i];
    }
    mTargets.clear();
}

void
DFBuildScene2D::SceneToObjectCoords(GUI::Point& ioPoint, int inKind) const {
    switch (inKind) {
    case vector:
        ioPoint.x *= mScalingX / 100.0;
        ioPoint.y *= mScalingY / 100.0;
        break;
    case point:
        ioPoint.x = (ioPoint.x / 100.0 - 0.5) * mScalingX + 0.5;
        ioPoint.y = (ioPoint.y / 100.0 - 0.5) * mScalingY + 0.5;
        break;
    default:
        throw bciexception("Unexpected coordinate kind: " << inKind);
    }
}

void
DFBuildScene2D::ObjectToSceneCoords(GUI::Point& ioPoint, int inKind) const {
    switch (inKind) {
    case vector:
        ioPoint.x *= 100.0 / mScalingX;
        ioPoint.y *= 100.0 / mScalingY;
        break;
    case point:
        ioPoint.x = (ioPoint.x - 0.5) / mScalingX * 100. + 50.0;
        ioPoint.y = (ioPoint.y - 0.5) / mScalingY * 100. + 50.0;
        break;
    default:
        throw bciexception("Unexpected coordinate kind: " << inKind);
    }
}

