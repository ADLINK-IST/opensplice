#ifndef _BOUNCINGSHAPEDYNAMICS_HPP
#define	_BOUNCINGSHAPEDYNAMICS_HPP

/** @file */
/**
 * @addtogroup demos_iShapes
 */
/** @{*/

#include <dds/dds.hpp>

#include <ShapeDynamics.hpp>
#include <QtCore/QRect>

// -- Shaped Include
#include <topic-traits.hpp>

namespace demo { namespace ishapes {
class BouncingShapeDynamics : public ShapeDynamics
{
public:
    BouncingShapeDynamics(int x0, int y0,
                          const QRect& shapeBounds,
                          const QRect& constraints,
                          float speed,
                          float angle,
                          const ShapeType& shape,
                          dds::pub::DataWriter<ShapeType> shapeWriter);

    virtual ~BouncingShapeDynamics();

public:
    virtual void simulate();

public:
    typedef ::dds::core::smart_ptr_traits<BouncingShapeDynamics>::ref_type ref_type;

private:
    BouncingShapeDynamics(const BouncingShapeDynamics& orig);
    BouncingShapeDynamics& operator=(const BouncingShapeDynamics&);

private:
    bool flip();

private:
    QRect shapeBounds_;
    float alpha_;
    float angle_;
    float speed_;
    ShapeType shape_;
    dds::pub::DataWriter<ShapeType>  shapeWriter_;
};
}
}

/** @}*/

#endif	/* _BOUNCINGSHAPEDYNAMICS_HPP */
