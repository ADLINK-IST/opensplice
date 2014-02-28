#ifndef DDS_DEMO_ISHAPES_WRITER_QOS_HPP_
#define DDS_DEMO_ISHAPES_WRITER_QOS_HPP_

/** @file */

#include <dds/dds.hpp>
#include <QtGui/QtGui>
#include <ui_writerQosForm.h>

/**
 * @addtogroup demos_iShapes
 */
/** @{*/

namespace demo { namespace ishapes {
class WriterQosDialog : public QDialog
{
    Q_OBJECT;
public:
    WriterQosDialog();
    virtual ~WriterQosDialog();

public:
    dds::pub::qos::DataWriterQos get_qos();

public slots:
    virtual void accept();
    virtual void reject();
private:
    Ui::WriterQoS qosForm_;
    dds::pub::qos::DataWriterQos qos_;
};
}
}

/** @}*/

#endif /* DDS_DEMO_ISHAPES_WRITER_QOS_HPP_ */
