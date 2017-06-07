#include <string.h>

#include "vortex_os.h"
#include "os_report.h"
#include "v_service.h"
#include "v_participant.h"
#include "v_publisher.h"
#include "v_writer.h"
#include "v_builtin.h"
#include "u__user.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"

static u_result
u__builtinWriterDeinitW(
    void *_this)
{
    return u__entityDeinitW (_this);
}

static void
u__builtinWriterFreeW(
    void *_this)
{
    u__entityFreeW (_this);
}

static u_result
u__builtinWriterInit(
    u_writer _this,
    const v_writer kw,
    const u_publisher publisher)
{
    return u_entityInit(u_entity(_this),v_entity(kw), u_observableDomain(u_observable(publisher)));
}

/* Each "networking" service that sees a given node (and possibly spliced) has
   it's own registration for the DCPSHeartbeat instance declaring that node.
   Only when all "networking" services have unregistered this instance will
   become not alive, indicating to whoever is interested that the node no
   longer exists (for example, triggering spliced to dispose it). Thus, it is
   necessary that all networking services have their own writers. This is
   merely a convenience function to ensure that all "networking" services use
   the same QoS settings. */
u_writer
u_builtinWriterNew(
    const u_publisher publisher,
    v_infoId infoId)
{
    u_writer _this = NULL;
    v_kernel kernel;
    u_result result = U_RESULT_OK;
    v_publisher vPublisher;
    v_service vService;
    v_topic vTopic = NULL;
    v_writer vBuiltinWriter, vWriter = NULL;
    v_writerQos vWriterQos = NULL;

    const c_char *vServiceName;
    const c_char *vInfoName;
    const os_char format[] = "%s %sWriter";
    os_char *name = NULL;
    os_size_t length;

    assert (publisher != NULL);

    result = u_observableWriteClaim(
        u_observable(publisher),(v_public*)(&vPublisher), C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        assert (vPublisher != NULL);
        kernel = v_objectKernel(vPublisher);

        /* Retrieve builtin DCPSHeartbeat writer created by spliced, so that
           the QoS can be copied. */
        vBuiltinWriter = v_builtinWriterLookup (kernel->builtin, infoId);
        assert (vBuiltinWriter != NULL);
        vWriterQos = v_writerGetQos(vBuiltinWriter);
        assert (vWriterQos != NULL);

        vTopic = v_writerTopic (vBuiltinWriter);
        assert (vTopic != NULL);

        vService = v_service (v_publisherParticipant (vPublisher));
        assert (vService != NULL);

        vServiceName = v_serviceGetName (vService);
        vInfoName = v_builtinInfoIdToName (infoId);

        length = strlen (format) + strlen (vServiceName) + strlen (vInfoName) + 1;
        name = os_malloc (length);
        (void)snprintf (name, length, format, vServiceName, vInfoName);
        vWriter = v_writerNew (vPublisher, name, vTopic, vWriterQos);
        if (vWriter != NULL) {
            _this = u_objectAlloc (sizeof (*_this), U_WRITER, u__builtinWriterDeinitW, u__builtinWriterFreeW);
            if (_this != NULL) {
                result = u__builtinWriterInit (_this, vWriter, publisher);
                if (result != U_RESULT_OK) {
                    const c_char message[] =
                    "Writer initialization failed. For DataWriter <%s>.";
                    OS_REPORT (
                               OS_ERROR, OS_FUNCTION, result, message, name);
                }
            } else {
                const c_char message[] =
                "Writer creation failed. For DataWriter: <%s>.";
                result = U_RESULT_OUT_OF_MEMORY;
                OS_REPORT (OS_ERROR, OS_FUNCTION, result, message, name);
            }

            if (result != U_RESULT_OK) {
                u_objectFree (u_object (_this));
                _this = NULL;
            }
        } else {
            const c_char message[] =
            "Kernel writer creation failed. For DataWriter: <%s>.";
            result = U_RESULT_OUT_OF_MEMORY;
            OS_REPORT (OS_ERROR, OS_FUNCTION, result, message, name);
        }

        c_free (vWriter);
        os_free (name);
        c_free (vWriterQos);

        (void)u_observableRelease(u_observable(publisher), C_MM_RESERVATION_HIGH);
    }

    return _this;
}


struct u__builtinHeartbeatArgument {
    c_ulong systemId;
    v_state state;
    v_writeResult result;
};

static void
u__builtinWriteFakeHeartbeat(
    v_public _this,
    void *varg)
{
    struct u__builtinHeartbeatArgument *arg = varg;

    /* Write a fake heartbeat with an infinite duration to prevent spliced
       from generating a fake heartbeat with a finite duration. It MUST
       remain local to the machine, therefore v_groupWrite must be used. */
    arg->result = v_builtinWriteHeartbeat(
        v_writer (_this), arg->systemId, os_timeWGet (), OS_DURATION_INFINITE, arg->state);
}

u_result
u_builtinWriteFakeHeartbeat(
    u_writer _this,
    c_ulong systemId,
    v_state state)
{
    struct u__builtinHeartbeatArgument arg;
    u_result result = U_RESULT_OK;

    assert(_this != NULL);

    arg.systemId = systemId;
    arg.state = state;
    arg.result = V_WRITE_ERROR;

    result = u_observableAction (
        u_observable (_this), &u__builtinWriteFakeHeartbeat, &arg);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernelWriteResult(arg.result);
    }

    return result;
}
