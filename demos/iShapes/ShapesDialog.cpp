/**
 * @file
 */
#include "config.hpp"
#include "ShapesDialog.hpp"
#include <QtGui/QtGui>
#include <iostream>
#include <Circle.hpp>
#include <Square.hpp>
#include <Triangle.hpp>
#include <BouncingShapeDynamics.hpp>
#include <DDSShapeDynamics.hpp>
#include <sstream>

namespace demo { namespace ishapes {
static const float PI = 3.1415926535F;

/* Ugly hack -- fixme */
static QColor  color_[CN];

const char* const colorString_[] =
{
    "BLUE",
    "RED",
    "GREEN",
    "ORANGE",
    "YELLOW",
    "MAGENTA",
    "CYAN",
    "GRAY",
    "BLACK"
};

static const std::string circleTopicName("Circle");
static const std::string squareTopicName("Square");
static const std::string triangleTopicName("Triangle");

#ifdef TESTBUILD
void ShapesDialog::CreatePublisher()
{
    onPublishButtonClicked();
    qDebug("Created Publisher");
}

void ShapesDialog::CreateSubscriber()
{
    onSubscribeButtonClicked();
    qDebug("Created Subscriber");
}

void ShapesDialog::CreatePublisherMMQos()
{
}
void ShapesDialog::CreateSubscriberMMQos(){}

void ShapesDialog::logShape()
{
}
void ShapesDialog::logDDSShape(){}

#endif

static std::string lexicalCast(int i)
{
    std::stringstream ss;
    ss << i;
    return ss.str();
}

ShapesDialog::ShapesDialog()
    :   timer(this), dp_(org::opensplice::domain::default_id())
{
    mainWidget.setupUi(this);
    shapesWidget = new ShapesWidget(mainWidget.renderFrame);
    shapesWidget->resize(mainWidget.renderFrame->size());
    shapesWidget->installEventFilter(this);
    shapesWidget->setObjectName(QString("renderFrame"));
    mainWidget.PausedLabel->setVisible(false);
    filterDialog_ = new FilterDialog(shapesWidget);
    connect(&timer, SIGNAL(timeout()),
            shapesWidget, SLOT(nextAnimationFrame()));

    color_[BLUE] = QColor(0x33, 0x66, 0x99);
    color_[RED] = QColor(0xCC, 0x33, 0x33);
    color_[GREEN] = QColor(0x99, 0xCC, 0x66);
    color_[ORANGE] = QColor(0xFF, 0x99, 0x33);
    color_[YELLOW] = QColor(0xFF, 0xFF, 0x66);
    color_[MAGENTA] = QColor(0xCC, 0x99, 0xCC);
    color_[CYAN] = QColor(0x99, 0xCC, 0xFF);
    color_[GRAY] = QColor(0x99, 0x99, 0x99);
    color_[BLACK] = QColor(0x33, 0x33, 0x33);
    timer.start(40);
}

ShapesDialog::~ShapesDialog()
{
}

void ShapesDialog::setDomainID(int DomainID)
{
    dds::domain::DomainParticipant temp(DomainID);
    qDebug() << "Set Domain ID to" << DomainID;
    dp_ = temp;
}

void ShapesDialog::setPartition(dds::core::StringSeq Partitions)
{
    gQos_ = dds::core::policy::Partition(Partitions);
    std::string s("Got Partitions:");
    for(unsigned int i = 0; i < Partitions.size(); i++)
    {
        s.append(" '" + Partitions[i] + "'");
    }
    qDebug("%s", s.c_str());
}

void
ShapesDialog::onPublishButtonClicked()
{
    dds::topic::qos::TopicQos topicQos = dp_.default_topic_qos()
                                         << dds::core::policy::Durability::Persistent()
                                         << dds::core::policy::DurabilityService()
                                            .service_cleanup_delay(dds::core::Duration(3600,0))
                                            .history_kind(dds::core::policy::HistoryKind::KEEP_LAST)
                                            .history_depth(DS_HISTORY)
                                            .max_samples(DS_MAX_SAMPLES)
                                            .max_instances(DS_MAX_INSTANCES)
                                            .max_samples_per_instance(DS_MAX_SAMPLES_X_INSTANCE);

    dds::pub::qos::PublisherQos PQos = dp_.default_publisher_qos()
                                        << gQos_;
    dds::pub::Publisher pub(dp_, PQos);


    int d = mainWidget.sizeSlider->value();
    float speed = ((float)mainWidget.speedSlider->value()) / 9;
    QRect rect(0, 0, d, d);
    // TODO: This should be retrieved from the canvas...


    QRect constr(0, 0, IS_WIDTH, IS_HEIGHT);
    // QRect constr = this->geometry();
    int x = constr.width() * ((float)rand() / RAND_MAX);
    int y = constr.height() * ((float)rand() / RAND_MAX);
    int cIdx = mainWidget.colorList->currentIndex();
    int sIdx = mainWidget.wShapeList->currentIndex();

    QBrush brush(color_[cIdx], Qt::SolidPattern);
    QPen pen(QColor(0xff, 0xff, 0xff), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    ShapeType shape;
    shape.color() = colorString_[cIdx];
    shape.shapesize() = rect.width();
    shape.x() = x;
    shape.y() = y;

    switch (sIdx)
    {

        case CIRCLE:
        {
            dds::topic::Topic<ShapeType> circle_(dp_, circleTopicName, topicQos);
            dds::pub::qos::DataWriterQos dwqos = circle_.qos();
            dds::pub::DataWriter<ShapeType> dw(pub, circle_, writerQos_.get_qos());


            BouncingShapeDynamics::ref_type dynamics(new BouncingShapeDynamics(x, y, rect, constr, PI/6, speed, shape, dw));
            Shape::ref_type  circle(new Circle(rect, dynamics, pen, brush));
            shapesWidget->addShape(circle);

            break;
        }

        case SQUARE:
        {
            dds::topic::Topic<ShapeType> square_(dp_, squareTopicName, topicQos);
            dds::pub::qos::DataWriterQos dwqos = square_.qos();
            dds::pub::DataWriter<ShapeType> dw(pub, square_, writerQos_.get_qos());


            BouncingShapeDynamics::ref_type dynamics(new BouncingShapeDynamics(x, y, rect, constr, PI/6, speed, shape, dw));
            Shape::ref_type  square(new Square(rect, dynamics, pen, brush));
            shapesWidget->addShape(square);
            break;
        }
        case TRIANGLE:
        {
            dds::topic::Topic<ShapeType> triangle_(dp_, triangleTopicName, topicQos);
            dds::pub::qos::DataWriterQos dwqos = triangle_.qos();
            dds::pub::DataWriter<ShapeType> dw(pub, triangle_, writerQos_.get_qos());


            BouncingShapeDynamics::ref_type dynamics(new BouncingShapeDynamics(x, y, rect, constr, PI/6, speed, shape, dw));
            Shape::ref_type  triangle(new Triangle(rect, dynamics, pen, brush));
            shapesWidget->addShape(triangle);
            break;
        }
        default:
            break;
    };
}

void
ShapesDialog::onSubscribeButtonClicked()
{
    dds::topic::qos::TopicQos topicQos = dp_.default_topic_qos()
                                         << dds::core::policy::Durability::Persistent()
                                         << dds::core::policy::DurabilityService()
                                            .service_cleanup_delay(dds::core::Duration(3600,0))
                                            .history_kind(dds::core::policy::HistoryKind::KEEP_LAST)
                                            .history_depth(DS_HISTORY)
                                            .max_samples(DS_MAX_SAMPLES)
                                            .max_instances(DS_MAX_INSTANCES)
                                            .max_samples_per_instance(DS_MAX_SAMPLES_X_INSTANCE);

    dds::sub::qos::SubscriberQos SQos = dp_.default_subscriber_qos() << gQos_;
    dds::sub::Subscriber sub(dp_, SQos);
    int d = mainWidget.sizeSlider->value();
    QRect rect(0, 0, d, d);
    QRect constr(0, 0, IS_WIDTH, IS_HEIGHT);
    int x = static_cast<int>(constr.width() * ((float)rand() / RAND_MAX)*0.9F);
    int y = static_cast<int>(constr.height() * ((float)rand() / RAND_MAX)*0.9F);
    int sIdx = mainWidget.rShapeList->currentIndex();

    QColor gray = QColor(0x99, 0x99, 0x99);
    QBrush brush(gray, Qt::SolidPattern);

    QPen pen(QColor(0xff,0xff,0xff), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    std::vector<std::string> empty;
    filterParams_ = empty;
    std::string filterS;
    if (filterDialog_->isEnabled())
    {
        QRect rect =  filterDialog_->getFilterBounds();
        std::string x0 = lexicalCast(rect.x());
        std::string x1 = lexicalCast(rect.x() + rect.width() -d);
        std::string y0 = lexicalCast(rect.y());
        std::string y1 = lexicalCast(rect.y() + rect.height() -d);
        filterParams_.push_back(x0);
        filterParams_.push_back(x1);
        filterParams_.push_back(y0);
        filterParams_.push_back(y1);
        filterS = "(x BETWEEN "
            + filterParams_[0]
            + " AND "
            + filterParams_[1]
            + ") AND (y BETWEEN "
            + filterParams_[2]
            + " AND "
            + filterParams_[3]
            + ")";

        if (filterDialog_->filterOutside() == false)
        {
             filterS = "(x < "
                 + filterParams_[0]
                 + " ) OR ( x > "
                 + filterParams_[1]
                 + " ) OR (y < "
                 + filterParams_[2]
                 + ") OR ( y > "
                 + filterParams_[3]
                 + ")";
        }
    }

    switch (sIdx)
    {

        case CIRCLE:
        {
            dds::topic::Topic<ShapeType> circle_(dp_, circleTopicName, topicQos);
	        dds::topic::ContentFilteredTopic<ShapeType> cfcircle_(dds::core::null);
            dds::sub::DataReader<ShapeType> dr(sub, circle_, readerQos_.get_qos());

            if (filterDialog_->isEnabled())
            {
                std::string tname = "CFCircle";
                const dds::topic::Filter filter(filterS);
                std::cout << filterS << std::endl;
                dds::topic::ContentFilteredTopic<ShapeType> cfcircle_(circle_, "CFCircle", filter);
	            dds::sub::DataReader<ShapeType> dr2(sub, cfcircle_, readerQos_.get_qos());
                dr = dr2;
            }
            for (int i = 0; i < CN; ++i)
            {
                std::string colorStr(colorString_[i]);

                DDSShapeDynamics::ref_type
                dynamics(new DDSShapeDynamics(x, y, dr, colorStr, i, true));

                Shape::ref_type
                circle(new Circle(rect, dynamics, pen, brush, true));

                dynamics->setShape(circle);
                shapesWidget->addShape(circle);
            }
            break;
        }

        case SQUARE:
        {
            dds::topic::Topic<ShapeType> square_(dp_, squareTopicName, topicQos);
            dds::topic::ContentFilteredTopic<ShapeType> cfsquare_(dds::core::null);
            dds::sub::DataReader<ShapeType> dr(sub, square_, readerQos_.get_qos());

            if (filterDialog_->isEnabled())
            {
                std::string tname = "CFSquare";
                const dds::topic::Filter filter(filterS);
                std::cout << filterS << std::endl;
                dds::topic::ContentFilteredTopic<ShapeType> cfsquare_(square_, "CFSquare", filter);
	            dds::sub::DataReader<ShapeType> dr2(sub, cfsquare_, readerQos_.get_qos());
                dr = dr2;
            }

            for (int i = 0; i < CN; ++i)
            {
                std::string colorStr(colorString_[i]);

                DDSShapeDynamics::ref_type
                dynamics(new DDSShapeDynamics(x, y, dr, colorStr, i, true));

                Shape::ref_type
                square(new Square(rect, dynamics, pen, brush, true));

                dynamics->setShape(square);
                shapesWidget->addShape(square);
            }
            break;
        }

        case TRIANGLE:
        {
            dds::topic::Topic<ShapeType> triangle_(dp_, triangleTopicName, topicQos);
            dds::topic::ContentFilteredTopic<ShapeType> cftriangle_(dds::core::null);
            dds::sub::DataReader<ShapeType> dr(sub, triangle_, readerQos_.get_qos());

            if (filterDialog_->isEnabled())
            {
                std::string tname = "CFTriangle";
                const dds::topic::Filter filter(filterS);
                std::cout << filterS << std::endl;
                dds::topic::ContentFilteredTopic<ShapeType> cftriangle_(triangle_, "CFTriangle", filter);
	            dds::sub::DataReader<ShapeType> dr2(sub, cftriangle_, readerQos_.get_qos());
                dr = dr2;
            }

            for (int i = 0; i < CN; ++i)
            {
                std::string colorStr(colorString_[i]);

                DDSShapeDynamics::ref_type
                dynamics(new DDSShapeDynamics(x, y, dr, colorStr, i, true));

                Shape::ref_type
                triangle(new Triangle(rect, dynamics, pen, brush, true));

                dynamics->setShape(triangle);
                shapesWidget->addShape(triangle);
            }
        break;
        }

        default:
            break;
    }
}

void
ShapesDialog::onReaderQosButtonClicked()
{
    readerQos_.setVisible(true);
}
void
ShapesDialog::onWriterQosButtonClicked()
{
    writerQos_.setVisible(true);
}

void
ShapesDialog::onFilterButtonClicked()
{
    filterDialog_->setVisible(true);
}
}}
