#include "PCHIncludes.h"
#pragma hdrstop

#include "GradientEllipticShape.h"
#include <QPainter>

using namespace std;
using namespace GUI;

Shape::TestResult
GradientEllipticShape::Contains(const GUI::Point& point) const {
    return EllipticShape::Contains(point);
}

Shape::TestResult
GradientEllipticShape::IntersectsArea(const Shape& shape) const {
    return EllipticShape::IntersectsArea(shape);
}

void
GradientEllipticShape::OnPaint(const GUI::DrawContext& inDC) {
    // Prepare the Brush
    QPainter* p = inDC.handle.painter;
    QRect drawRect(
        static_cast<int>(Rect().left),
        static_cast<int>(Rect().top),
        static_cast<int>(Rect().right - Rect().left),
        static_cast<int>(Rect().bottom - Rect().top)
    );

    float r = (Rect().right - Rect().left) / 2;
    float cx = Rect().left + r;
    float cy = Rect().top + r;

    // Center and focal point are equal
    QRadialGradient fillGradient(cx, cy, r, cx, cy);

    if (this->FillColor() != RGBColor(RGBColor::NullColor)) {
        fillGradient.setColorAt(0, this->FillColor());
        fillGradient.setColorAt(1, Qt::black);
    }

    QBrush fillBrush(fillGradient);
    p->setBrush(fillBrush);

    // Prepare the Pen
    QPen outlinePen;
    outlinePen.setStyle(Qt::NoPen);
    p->setPen(outlinePen);

    // Draw the rectangle
    p->drawEllipse(drawRect);
}
