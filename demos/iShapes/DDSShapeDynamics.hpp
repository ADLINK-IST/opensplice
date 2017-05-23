#ifndef _DDSSHAPEDYNAMICS_HPP
#define	_DDSSHAPEDYNAMICS_HPP

/** @file */

#include <dds/dds.hpp>
#include <ShapeDynamics.hpp>
#include <QtCore/QRect>
#include <QtGui/QtGui>
#include <Shape.hpp>
// -- Shaped Include
#include <topic-traits.hpp>

namespace demo { namespace ishapes {
#define CN 9
typedef ::dds::core::smart_ptr_traits<Shape>::ref_type SharedShape;
typedef ::dds::core::smart_ptr_traits<Shape>::weak_ref_type WeakSharedShape;

/**
 * @addtogroup demos_iShapes
 */
/** @{*/
class DDSShapeDynamics : public ShapeDynamics
{
public:
    enum
    {
        BLUE    = 0,
        RED     = 1,
        GREEN   = 2,
        ORANGE  = 3,
        YELLOW  = 4,
        MAGENTA = 5,
        CYAN    = 6,
        GRAY    = 7,
        BLACK   = 8
    };

public:
    DDSShapeDynamics(
        int x0, int y0,
        dds::sub::DataReader<ShapeType> shapeReader,
        const std::string& color,
        int colorIdx,
        bool dummy
    );

    virtual ~DDSShapeDynamics();

public:
    typedef ::dds::core::smart_ptr_traits<DDSShapeDynamics>::ref_type ref_type;

public:

    void setShape(SharedShape shape)
    {
        shape_ = shape;
    }


    virtual void simulate();

private:
    //DDSShapeDynamics(const DDSShapeDynamics& orig);
    WeakSharedShape shape_;
    int x0_;
    int y0_;
    dds::sub::DataReader<ShapeType>  shapeReader_;
    bool attached_;
    std::string color_;
    int colorIdx_;
    QColor  colorList_[CN];
    bool updateBounds_;
    dds::sub::LoanedSamples<ShapeType> samples;
    bool dummy_;

};
}
}

/** @}*/

#endif	/* _DDSSHAPEDYNAMICS_HPP */
