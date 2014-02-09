#ifndef DFBUILD_SCENE_H
#define DFBUILD_SCENE_H

#include "Environment.h"
#include "Color.h"
#include <string>

class DFBuildScene : protected Environment {
public:
    DFBuildScene() {}
    virtual ~DFBuildScene() {}

    virtual self& Initialize();

    virtual float CursorRadius() const;
    virtual float CursorXPosition() const;
    virtual float CursorYPosition() const;
    virtual float CursorZPosition() const;
    virtual DFBuildScene& SetCursorPosition(float x, float y, float z);
    virtual DFBuildScene& SetCursorVisible(bool);
    virtual DFBuildScene& SetCursorColor(RGBColor);

    virtual int NumTargets() const;
    virtual bool TargetHit(int) const;
    virtual float CursorTargetDistance(int) const;
    virtual DFBuildScene& SetTargetVisible(bool, int);
    virtual DFBuildScene& SetTargetColor(RGBColor, int);
};

#endif // DFBUILD_SCENE_H
