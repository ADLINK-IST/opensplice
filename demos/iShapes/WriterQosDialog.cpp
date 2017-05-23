/**
 * @file
 */
#include <WriterQosDialog.hpp>
#include <iostream>

namespace demo { namespace ishapes {

WriterQosDialog::WriterQosDialog()
{
    qosForm_.setupUi(this);
    this->setVisible(false);
}

WriterQosDialog::~WriterQosDialog() { }

void
WriterQosDialog::accept()
{
    this->setVisible(false);
}

void
WriterQosDialog::reject()
{
    this->setVisible(false);
}

dds::pub::qos::DataWriterQos
WriterQosDialog::get_qos()
{
    dds::pub::qos::DataWriterQos tmpQos;

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

    qos_ << dds::core::policy::TransportPriority(qosForm_.prioritySpinBox->value());

    if (qosForm_.ownershipExclusiveRButt->isChecked())
    {
        qos_ << dds::core::policy::Ownership::Exclusive();
        qos_ << dds::core::policy::OwnershipStrength(qosForm_.strengthSpinBox->value());
    }

    if (qosForm_.keepLastWButton->isChecked())
    {
        qos_ << dds::core::policy::History::KeepLast(qosForm_.depthSpinBoxW->value());
    }
    else
    {
        dds::core::policy::History h = dds::core::policy::History::KeepAll();
        h.depth(-1);
        qos_ << h;
    }
    return qos_;
}
}}
