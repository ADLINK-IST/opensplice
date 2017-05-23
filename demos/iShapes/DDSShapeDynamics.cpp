/**
 * @file
 */
#include <dds/core/types.hpp>
#include "DDSShapeDynamics.hpp"
#include <iostream>
#ifdef TESTBUILD
    #include "os_time.h"
#endif

namespace demo { namespace ishapes {
extern char* colorString_[];


DDSShapeDynamics::DDSShapeDynamics(int x0, int y0,
                                   dds::sub::DataReader<ShapeType> shapeReader,
                                   const std::string& color,
                                   int colorIdx, bool dummy)
    :   ShapeDynamics(x0, y0, QRect(0, 0, 0, 0)),
        x0_(x0),
        y0_(y0),
        shapeReader_(shapeReader),
        attached_(false),
        color_(color),
        colorIdx_(colorIdx),
        updateBounds_(true),
        dummy_(dummy)
{
    colorList_[BLUE] = QColor(0x33, 0x66, 0x99);
    colorList_[RED] = QColor(0xCC, 0x33, 0x33);
    colorList_[GREEN] = QColor(0x99, 0xCC, 0x66);
    colorList_[ORANGE] = QColor(0xFF, 0x99, 0x33);
    colorList_[YELLOW] = QColor(0xFF, 0xFF, 0x66);
    colorList_[MAGENTA] = QColor(0xCC, 0x99, 0xCC);
    colorList_[CYAN] = QColor(0x99, 0xCC, 0xFF);
    colorList_[GRAY] = QColor(0x99, 0x99, 0x99);
    colorList_[BLACK] = QColor(0x33, 0x33, 0x33);
}

DDSShapeDynamics::~DDSShapeDynamics()
{
}

void
DDSShapeDynamics::simulate()
{

    QPoint tmp;

    //samples::iterator sample;
    samples = shapeReader_.read();
    plist_.erase(plist_.begin(), plist_.end());

    //Create a dummy when the subscribe button is clicked until the first sample arrives
    if(dummy_ == true)
    {
        SharedShape shape;
        if (shape = shape_.lock())
        {
            plist_.push_back(QPoint((IS_WIDTH/2)-10, (IS_HEIGHT/2)-10));
            QBrush brush = QBrush(QColor(0xd3, 0xd3, 0xd3), Qt::SolidPattern);
            shape->setBrush(brush);
            QRect bounds(0, 0, 90, 90);
        }
    }

    for (dds::sub::LoanedSamples<ShapeType>::const_iterator sample = samples.begin(); sample != samples.end(); ++sample)
    {
        dummy_ = false;
        #ifdef TESTBUILD
        qDebug() << "Time:"
                 << os_timeGet().tv_sec
                 << os_timeGet().tv_nsec
                 << "Colour:"
                 << (*sample).data().color().c_str()
                 << "Size:" << (*sample).data().shapesize()
                 << "x:" << (*sample).data().x()
                 << "y:" << (*sample).data().y();
        #endif
        SharedShape shape;
        if (shape = shape_.lock())
        {
          if (strcmp((*sample).data().color().c_str(), color_.c_str()) == 0)
          {
              tmp.rx() = (*sample).data().x();
              tmp.ry() = (*sample).data().y();
              plist_.push_back(tmp);

              if (attached_ == false)
              {
                  attached_ = true;
                  QBrush brush = QBrush(colorList_[colorIdx_], Qt::SolidPattern);
                  shape->setBrush(brush);
              }
              QRect bounds(0, 0, (*sample).data().shapesize(), (*sample).data().shapesize());
              shape->setBounds(bounds);
          }
        }
    }
}
}}
