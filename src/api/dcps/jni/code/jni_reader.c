/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "c_base.h"
#include "q_expr.h"
#include "v_dataReader.h"
#include "v_readerQos.h"
#include "u_user.h"
#include "u_query.h"
#include "u_dataReader.h"
#include "u_reader.h"
#include "sd_serializerXML.h"
#include "sd_serializer.h"
#include "jni__handler.h"
#include "jni_subscriber.h"
#include "jni_participant.h"
#include "jni_topic.h"
#include "jni_reader.h"
#include "jni_misc.h"
#include "os.h"
#include "os_report.h"
#include "os_heap.h"

static v_actionResult jni_readerSerializeData(c_object o, c_voidp copyArg);

jni_reader
jni_readerNew(
    jni_subscriber sub,
    jni_topicDescription top,
    v_readerQos qos)
{
    jni_reader reader;
    char *expr;
    const char *select = "select * from ";
    q_expr qexpr;
    size_t size;

    assert(sub);
    assert(sub->participant);
    assert(sub->usubscriber);
    assert(sub->participant->uparticipant);
    assert(top);
    assert(top->name);

    size = strlen(top->name) + strlen(select) + 1;
    if((expr = os_malloc(size)) == NULL){
        goto err_qexpr;
    }

    (void) snprintf(expr, size, "%s%s", select, top->name);
    qexpr = q_parse(expr);
    os_free(expr);
    if(qexpr == NULL){
        goto err_qexpr;
    }

    if((reader = os_malloc(sizeof *reader)) == NULL){
        OS_REPORT_1(OS_ERROR, "jni_readerNew", 0,
                    "Memory claim of %" PA_PRIuSIZE " denied.",
                    sizeof *reader);
        goto err_malloc;
    }

    if((reader->ureader = u_dataReaderNew(sub->usubscriber, NULL, qexpr, NULL, qos, TRUE)) == NULL){
        /* Error reported by u_dataReaderNew */
        goto err_ureader;
    }

    reader->subscriber = sub;
    reader->description = top;
    reader->uquery = NULL;

    q_dispose(qexpr);

    return reader;

/* Error handling */
err_ureader:
    os_free(reader);
err_malloc:
    q_dispose(qexpr);
err_qexpr:
    return NULL;
}

jni_result
jni_readerFree(
    jni_reader rea)
{
    jni_result r;

    r = JNI_RESULT_OK;

    if(rea != NULL){
        if((r == JNI_RESULT_OK) && (rea->uquery != NULL)){
            r = jni_convertResult(u_queryFree(rea->uquery));
        }
        if((r == JNI_RESULT_OK) && (rea->ureader != NULL)){
            r = jni_convertResult(u_dataReaderFree(rea->ureader));
        }
        if(r == JNI_RESULT_OK){
            os_free(rea);
        }
    } else{
        r = JNI_RESULT_BAD_PARAMETER;
    }
    return r;
}

struct jni_readerArg {
    c_char* result;
};

c_char*
jni_readerRead(
    jni_reader rea)
{
    struct jni_readerArg arg;

    arg.result = NULL;

    if(rea != NULL){
        if(rea->uquery != NULL){
            u_queryRead(rea->uquery, jni_readerSerializeData, &arg);
        }
        else if(rea->ureader != NULL){
            u_dataReaderRead(rea->ureader, jni_readerSerializeData, &arg);
        }
        else{
          /*Will not happen.*/
        }
    }
    return arg.result;
}

c_char*
jni_readerTake(
    jni_reader rea)
{
    struct jni_readerArg arg;

    arg.result = NULL;

    if(rea != NULL){
        if(rea->uquery != NULL){
           u_queryTake(rea->uquery, jni_readerSerializeData, &arg);
        }
        else if(rea->ureader != NULL){
           u_dataReaderTake(rea->ureader, jni_readerSerializeData, &arg);
        }
        else{
          /*Will not happen.*/
        }
    }
    return arg.result;
}

jni_result
jni_readerSetQuery(
    jni_reader rea,
    const c_char* query_expression,
    c_value params[])
{
    q_expr expr;
    jni_result r;
    u_query query;

    r = JNI_RESULT_OK;

    if((rea != NULL) && (rea->ureader != NULL)){
        expr = q_parse(query_expression);

        if(expr == NULL){
            r = JNI_RESULT_ERROR;
        }
        else{
            query = u_queryNew(u_reader(rea->ureader), NULL, expr, params);
            q_dispose(expr);

            if(query == NULL){
                r = JNI_RESULT_ERROR;
            }
            else if(rea->uquery == NULL){
                rea->uquery = query;
            }
            else{ /*A query exists already, free it before setting the new one.*/
                r = jni_convertResult(u_queryFree(rea->uquery));

                if(r == JNI_RESULT_OK){
                    rea->uquery = query;
                }
                else{
                    u_queryFree(query);
                }
            }
        }
    }
    else{
        r = JNI_RESULT_BAD_PARAMETER;
    }
    return r;
}

static v_actionResult
jni_readerSerializeData(
    c_object o,
    c_voidp copyArg)
{
    sd_serializer ser;
    sd_serializedData data;
    struct jni_readerArg *arg;

    arg = (struct jni_readerArg *)copyArg;

    if(o != NULL){
        ser = sd_serializerXMLNewTyped(c_getType(o));
        data = sd_serializerSerialize(ser, o);
        arg->result = sd_serializerToString(ser, data);

        sd_serializedDataFree(data);
        sd_serializerFree(ser);
    }
    return 0;
}
