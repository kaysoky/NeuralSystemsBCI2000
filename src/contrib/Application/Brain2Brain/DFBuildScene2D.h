#ifndef DFBUILD_SCENE_2D_H
#define DFBUILD_SCENE_2D_H

#include "DFBuildScene.h"
#include "Shapes.h"
#include "DisplayWindow.h"

class DFBuildScene2D : public DFBuildScene {
typedef DFBuildScene2D self;
public:
    DFBuildScene2D(GUI::DisplayWindow&);
    virtual ~DFBuildScene2D();

    virtual self& Initialize();

    virtual float CursorRadius() const;
    virtual float CursorXPosition() const;
    virtual float CursorYPosition() const;
    virtual float CursorZPosition() const;
    virtual self& SetCursorPosition(float x, float y, float z);
    virtual self& SetCursorVisible(bool);
    virtual self& SetCursorColor(RGBColor);

    virtual int NumTargets() const;
    virtual bool TargetHit(int) const;
    virtual float CursorTargetDistance(int) const;
    virtual self& SetTargetVisible(bool, int);
    virtual self& SetTargetColor(RGBColor, int);

private:
    void ClearObjects();
    enum {point, vector};
    void SceneToObjectCoords(GUI::Point&, int kind) const;
    void ObjectToSceneCoords(GUI::Point&, int kind) const;

    GUI::DisplayWindow& mDisplay;
    float mScalingX,
          mScalingY,
          mCursorZ;
    EllipticShape* mpCursor;
    RectangularShape* mpBoundary;
    std::vector<EllipticShape*> mTargets;
    std::vector<EllipticShape*> cTargets;
};

#endif // DFBUILD_SCENE_2D_H
