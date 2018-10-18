#include "dds.h"
#include "HelloWorldData.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char ** argv)
{
    dds_entity_t topic;
    dds_entity_t writer;
    int status;
    HelloWorldData_Msg msg;

    /* Create a Topic. */
    /* Passing NULL as first argument will result in an implicitly created participant */
    status = dds_topic_create (NULL, &topic, &HelloWorldData_Msg_desc, "HelloWorldData_Msg", NULL, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a Writer. */
    /* Passing NULL as first argument will result in an implicitly created publisher */
    status = dds_writer_create (NULL, &writer, topic, NULL, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    printf("=== [Publisher]  Waiting for a reader to be discovered ...\n");

    status = dds_status_set_enabled(writer, DDS_PUBLICATION_MATCHED_STATUS);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    while(true)
    {
      uint32_t ret;
      ret = dds_status_changes (writer);

      if (ret == DDS_PUBLICATION_MATCHED_STATUS) {
        break;
      }
      /* Polling sleep. */
      dds_sleepfor (DDS_MSECS (20));
    }

    /* Create a message to write. */
    msg.userID = 1;
    msg.message = "Hello World";

    printf ("=== [Publisher]  Preparing : ");
    printf ("Message (%d, %s)\n", msg.userID, msg.message);

    status = dds_write (writer, &msg);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    printf ("=== [Publisher]  Message written.\n");

    dds_entity_delete (topic);
    dds_entity_delete (writer);

    return EXIT_SUCCESS;
}
