#ifndef DDS_DEMO_ISHAPES_READER_QOS_HPP_
#define DDS_DEMO_ISHAPES_READER_QOS_HPP_

/** @file */
/**
 * @addtogroup demos_iShapes
 */
/** @{*/

#include <dds/dds.hpp>
#include <QtGui/QtGui>
#include <ui_readerQosForm.h>

namespace demo { namespace ishapes {
class ReaderQosDialog : public QDialog
{
    Q_OBJECT;
public:
    ReaderQosDialog();
    virtual ~ReaderQosDialog();

public:
    dds::sub::qos::DataReaderQos get_qos();

public slots:
    virtual void accept();
    virtual void reject();

private:
    Ui::ReaderQos qosForm_;
    dds::sub::qos::DataReaderQos qos_;
};
}
}

/** @}*/

#endif /* DDS_DEMO_ISHAPES_READER_QOS_HPP_ */
