/* Interface */
#include "nw__writer.h"
#include "nw_writer.h"

/* Implementation */
#include "os.h"

/* Public methods */

nw_bool
nw_writerWriteMessage(
    nw_writer writer,
    v_networkReaderEntry entry, 
    v_message message,
    c_ulong messageId,
    v_gid sender,
    c_bool sendTo,
    v_gid receiver)
{
    nw_bool result = FALSE;
    if (writer != NULL) {
        if (writer->writeMessageFunc != NULL) {
            result = writer->writeMessageFunc(writer,
                entry, message, messageId, sender, sendTo, receiver);
        }
    }
    return result;
}

void
nw_writerFree(
    nw_writer writer)
{
    if (writer != NULL) {
        if (writer->finalizeFunc != NULL) {
            writer->finalizeFunc(writer);
        }
        os_free(writer);
    }    
}


/* Protected methods */

void
nw_writerInitialize(
    nw_writer writer,
    nw_writerWriteMessageFunc writeMessageFunc,
    nw_writerFinalizeFunc finalizeFunc)
{
    if (writer) {
        writer->writeMessageFunc = writeMessageFunc;
        writer->finalizeFunc = finalizeFunc;
    }
}

