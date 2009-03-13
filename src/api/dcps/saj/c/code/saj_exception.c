
#include "saj_utilities.h"
#include "saj__exception.h"

void
saj_exceptionThrow (
    JNIEnv *env,
    int errorCode,
    const char *format,
    ...)
{
    char memo[512];
    va_list valist;
    jstring jmemo;
    jthrowable jexception;

    va_start (valist, format);
    vsnprintf (memo, sizeof(memo), format, valist);
    va_end (valist);
    jmemo = (*env)->NewStringUTF (env, memo);
    if (jmemo) {
	jexception = (*env)->CallStaticObjectMethod (
	    env,
	    GET_CACHED(utilities_class),
	    GET_CACHED(utilities_throwException_mid),
	    (jint)errorCode,
	    jmemo);
	(*env)->Throw (env, jexception);
	(*env)->DeleteLocalRef (env, jexception);
	(*env)->DeleteLocalRef (env, jmemo);
    }
}
