#ifndef _CIRCLE_HPP
#define _CIRCLE_HPP

/** @file */
/**
 * @addtogroup demos_iShapes
 */
/** @{*/

#include <ShapeDynamics.hpp>
#include <Shape.hpp>

namespace demo { namespace ishapes {
class Circle : public Shape
{
public:

    Circle(const QRect& bounds,
           ShapeDynamics::ref_type dynamics,
           const QPen& pen,
           const QBrush& brush,
           bool targeted = false);

public:
    virtual void update();
    virtual void paint(QPainter& painter);
private:
    Circle(const Circle&);
    Circle& operator=(Circle&);

private:
    ShapeDynamics::ref_type dynamics_;
};
}
}

/** @}*/

#endif	/* _CIRCLE_HPP */
