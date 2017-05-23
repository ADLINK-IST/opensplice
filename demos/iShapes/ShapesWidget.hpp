#ifndef _SHAPESWIDGET_HPP
#define	_SHAPESWIDGET_HPP

/** @file */
/**
 * @addtogroup demos_iShapes
 */
/** @{*/

#include <QtGui/QWidget>
#include <QtCore/QRect>
#include <QtGui/QPixmap>
#include <vector>

#include <Shape.hpp>

namespace demo { namespace ishapes {
class ShapesWidget  : public QWidget
{
    Q_OBJECT
public:
    typedef std::vector<Shape::ref_type > ShapeList;
    typedef std::vector<QRect> FilterList;
public:

    ShapesWidget(QWidget *parent = 0);
    virtual ~ShapesWidget();

public:
    void addFilter(const QRect& filter);
    void displayFilter(const QRect& currentFilter);
    bool paused()
    {
        return paused_;
    }
    void paused(const bool p)
    {
        paused_ = p;
    }

public slots:
    void nextAnimationFrame();
    void addShape(Shape::ref_type shape);

protected:
    void paintEvent(QPaintEvent *event);

private:
    ShapesWidget(const ShapesWidget& orig);

private:
    ShapeList shapeList_;
    FilterList filterList_;
    QRect currentFilter_;
    bool showCurrentFilter_;
    QPixmap logo_;
    QPixmap ptpcm_;
    QPixmap simd_;
    bool paused_;
};
}
}

/** @}*/

#endif	/* _SHAPESWIDGET_HPP */
