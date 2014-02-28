/**
 * @file
 */
#include <ReaderQosDialog.hpp>
#include <iostream>

namespace demo { namespace ishapes {

ReaderQosDialog::ReaderQosDialog()
{
    qosForm_.setupUi(this);
    this->setVisible(false);
}

ReaderQosDialog::~ReaderQosDialog() { }

void
ReaderQosDialog::accept()
{
    this->setVisible(false);
}

void
ReaderQosDialog::reject()
{
    this->setVisible(false);
}

dds::sub::qos::DataReaderQos
ReaderQosDialog::get_qos()
{
    dds::sub::qos::DataReaderQos tmpQos;

    qos_ = tmpQos;

    if (qosForm_.reliableRButt->isChecked())
        qos_ << dds::core::policy::Reliability::Reliable();

    switch (qosForm_.durabilityComboBox->currentIndex())
    {
        case 0:
            qos_ << dds::core::policy::Durability::Volatile();
            break;
        case 1:
            qos_ << dds::core::policy::Durability::TransientLocal();
            break;
        case 2:
            qos_ << dds::core::policy::Durability::Transient();
            break;
        case 3:
            qos_ << dds::core::policy::Durability::Persistent();
            break;
    };

    if (qosForm_.exclusiveRButt->isChecked())
        qos_ << dds::core::policy::Ownership::Exclusive();
    if (qosForm_.keepLastRButton->isChecked())
    {
        qos_ << dds::core::policy::History::KeepLast(qosForm_.depthSpinBox->value());
    }
    else
    {
        dds::core::policy::History h = dds::core::policy::History::KeepAll();
        h.depth(-1);
        qos_ << h;
    }
    if (qosForm_.timeBasedFilterActive->isChecked()) {
        int64_t period = qosForm_.timeBasedFilterValue->text().toInt();
        dds::core::Duration d;
        qos_ << dds::core::policy::TimeBasedFilter(d.from_millisecs(period));
  }


    return qos_;
}
}}
