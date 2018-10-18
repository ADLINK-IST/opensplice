/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */


package org.opensplice.dds.dcps;

/**
 * Implementation of the ReportStack.
 */
public class ReportStack {

    /**
     * Constructor is private. ReportStack objects cannot be created, it's
     * simply a wrapper around the ReportStack operations implemented in
     * C. A ReportStack should be create implicitly by invoking the static
     * start() method.
     */
    private ReportStack() { }

    /**
     * Pass an error to the currently active ReportStack.
     */
    public static void report(int code, String message)
    {
        int line = 0;
        String file = null;
        String method = null;

        StackTraceElement trace = Thread.currentThread().getStackTrace()[2];

        file = trace.getFileName();
        if (file == null) {
            /* When not compiled with debugging flags, the filename may return
               null. In that case we deduct the fileName from its classname,
               which is in our case almost always correct. */
            file = trace.getClassName() + ".java";
        }
        line = trace.getLineNumber();
        method = trace.getClassName() + "." + trace.getMethodName();

        jniReport(file, line, method, code, message);
    }

    /**
     * Pass an error to the currently active ReportStack.
     */
    public static void deprecated(String message)
    {
        int line = 0;
        String file = null;
        String method = null;

        StackTraceElement trace = Thread.currentThread().getStackTrace()[2];

        file = trace.getFileName();
        if (file == null) {
            /* When not compiled with debugging flags, the filename may return
               null. In that case we deduct the fileName from its classname,
               which is in our case almost always correct. */
            file = trace.getClassName() + ".java";
        }
        line = trace.getLineNumber();
        method = trace.getClassName() + "." + trace.getMethodName();

        jniReportDeprecated(file, line, method, message);
    }

    /**
     * Flush available error reports to the active error devices when the
     * error condition passed as a parameter resolves to true. This function
     * also deactivates the current ReportStack, regardless of whether any
     * reports have been flushed. Re-activate the ReportStack by invoking
     * the static start() method.
     */
    public static void flush(boolean flush)
    {
        flushReport(null, flush);
    }

    public static void flush(ObjectImpl obj, boolean flush)
    {
        flushReport(obj, flush);
    }

    private static void flushReport(ObjectImpl obj, boolean flush)
    {
        int line = 0;
        String file = null;
        String method = null;

        if (jniReportFlushRequired(flush) != 0) {
            StackTraceElement trace = Thread.currentThread().getStackTrace()[3];
            int domainId = (obj != null) ? obj.getDomainId() : DDS.DOMAIN_ID_INVALID.value;

            file = trace.getFileName();
            if (file == null) {
                /* See comment in ReportStack.report above. */
                file = trace.getClassName() + ".java";
            }
            method = trace.getClassName() + "." + trace.getMethodName();
            line = trace.getLineNumber();

            jniFlush(file, line, method, flush, domainId);
        }
    }

    /**
     * Starts a new ReportStack for the given thread. ReportStack objects are
     * only created once per thread. Subsequent invocations of the start()
     * increment the ReportStack counter.
     *
     * This function is implemented directly as a Java Native Interface
     * function to cut as much overhead as possible.
     */
    public static native void start();

    public static native void jniReport(
        String file,
        int line,
        String method,
        int code,
        String message);

    public static native void jniReportDeprecated(
            String file,
            int line,
            String method,
            String message);

    public static native int jniReportFlushRequired(
        boolean flush);

    public static native void jniFlush(
        String file,
        int line,
        String method,
        boolean flush,
        int domainId);
}
