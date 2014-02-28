#ifndef _SHAPE_HPP
#define	_SHAPE_HPP

/** @file */
/**
 * @addtogroup demos_iShapes
 */
/** @{*/

#include <QtCore/QRect>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <dds/core/refmacros.hpp>

namespace demo { namespace ishapes {
class Shape
{
public:
    Shape(const QRect& bounds,
          const QPen& pen,
          const QBrush& brush,
          bool targeted = false);

    virtual ~Shape();
public:
    virtual void update() = 0;
    virtual void paint(QPainter& painter) = 0;

public:
    virtual void setPen(const QPen& pen);
    virtual void setBrush(const QBrush& brush);
    virtual void setBounds(const QRect& bounds);
    virtual void set_targeted(bool b);

public:
    typedef ::dds::core::smart_ptr_traits<Shape>::ref_type ref_type;


private:
    Shape(const Shape&);
    Shape& operator=(const Shape&);

protected:
    QRect bounds_;
    QPen pen_;
    QBrush brush_;
    bool targeted_;
};
}
}

/** @}*/

#endif	/* _SHAPE_HPP */
