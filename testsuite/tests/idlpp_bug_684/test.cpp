#include "ccpp_TestTopics.h"
#include "ccpp_dds_dcps.h"
#include <string>

int main (int, char *[])
{
    {
    DDS::UserDataQosPolicy uQos;

    std::string text = "abcde";
    DDS::ULong size = text.size();
    uQos.value.length(size);		// memory allocated here? (by my .exe)
    for (DDS::ULong i = 0; i < size; i++) {
        uQos.value[i] = text[i];
    }
    } // uQos goes out of scope here - OSPL DLL seems to try to free the memory.


    /*
    If this test compiles, it's passed
    */
    return 0;
}
