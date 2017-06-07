#include <ShapesDialog.hpp>
#include <QtGui/QApplication>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#ifdef _WIN32
	#include <Windows.h>
#endif
#ifdef TESTBUILD
    #include <QtTest/QtTest>
#endif

/**
 * @addtogroup demos_iShapes ISO C++ DCPS iShapes Demonstrator Application
 * Refer to @ref ishapes_readme for information on building and running
 * this demo.
 */
/** @{*/
/** @dir */
/** @file */
namespace demo { namespace ishapes {
class iShapes : public QApplication {
    public:
        iShapes(int& argc, char ** argv) :
        QApplication(argc, argv) { }
        virtual ~iShapes() { }

        // reimplemented from QApplication so we can throw exceptions in slots
        virtual bool notify(QObject * receiver, QEvent * event)
        {
            try
            {
                return QApplication::notify(receiver, event);
            }
            catch(std::exception& e)
            {
                std::string message = "Exception caught:-\n";
                message += e.what();
                QString qmessage(message.c_str());
                QMessageBox::critical(NULL, "Error", qmessage);
            }
            return false;
        }
};
}}
int main(int argc, char *argv[])
{
    srand(clock());
    demo::ishapes::iShapes app(argc, argv);
    #ifdef TESTBUILD
    /** @todo Simon - sort this out */
    Q_INIT_RESOURCE(ishape_qrc);
    #else
    Q_INIT_RESOURCE(ishape);
    #endif

    int retval;
    try
    {
        demo::ishapes::ShapesDialog shapes;
        #ifndef TESTBUILD
        if(argc > 1)
        {
            dds::core::StringSeq partitions;
            for(int i = 1; i < argc; i++)
            {
                if(isdigit(argv[i][0]))
                    shapes.setDomainID(atoi(argv[i]));
                else
                    partitions.push_back(std::string(argv[i]));
            }
            if(partitions.size() > 0)
                shapes.setPartition(partitions);
        }
        #endif
        #ifdef TESTBUILD
            //Start publishing a circle with default QoS
            QTest::qExec(&shapes, argc, argv);
        #else
				#ifdef _WIN32
				FreeConsole();
				#endif
                shapes.show();
        #endif
        retval = app.exec();

    }
    catch(const dds::core::Exception& e)
    {
        std::string message = "Exception caught:-\n";
        message += e.what();
        QString qmessage(message.c_str());
        QMessageBox::critical(NULL, "Error", qmessage);
        retval = 1;
    }

    return retval;
}

/** @}*/
