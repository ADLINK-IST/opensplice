package org.openorb.compiler.parser;

public interface ErrorHandler
{

    public void error(
        String fileName,
        int line,
        String message);

    public void warning(
        String fileName,
        int line,
        String message);

    /* Invoked when the maximum number of errors is exceeded, after which
     * compilation of the IDL file is stopped.
     */
    public void maxErrorsExceeded(
        String fileName,
        int maxErrors);

}
