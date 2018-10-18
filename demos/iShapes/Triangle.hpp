#ifndef _TRIANGLE_HPP
#define _TRIANGLE_HPP
/** @file */

#include <Shape.hpp>
#include <ShapeDynamics.hpp>
/**
 * @addtogroup demos_iShapes
 */
/** @{*/

namespace demo { namespace ishapes {
class Triangle : public Shape
{
public:
    Triangle(const QRect& bounds,
             ShapeDynamics::ref_type dynamics,
             const QPen& pen,
             const QBrush& brush,
             bool targeted = false);

    virtual ~Triangle();

public:
    virtual void update();
    virtual void paint(QPainter& painter);
    virtual void setBounds(const QRect& bounds);

private:
    Triangle(const Triangle&);
    Triangle& operator=(const Triangle&);

private:
    ShapeDynamics::ref_type dynamics_;
    QPolygon triangle_;
};
}
}

/** @}*/

#endif	/* _TRIANGLE_HPP */
